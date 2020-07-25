//
// Created by dxy on 2020/7/23.
//

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Ccompiler.h"

/**
 * DFA
 *
 * transition map
 * 1--(letter)-->2--(letter(circle))-->2
 * 1--(digit)-->3--(digit(circle))-->3
 * 1--(!|+|%|=)-->4--(=)-->5
 * 1--(<)-->6--(<)-->7
 *           --(=)-->8
 * 1--(>)-->9--(>)-->10
 *           --(=)-->11
 * 1--(-)-->12--(>)-->13
 *            --(=)-->14
 * 1--(.|,|;|'|{|}|[|]|(|)|#|:)-->15
 * 1--(*)-->16--(*(circle))-->16--(=)-->17
 * 1--(/)-->18--(=)-->19
 *            --(*)-->20
 *            --(/)-->21
 * 1--(&)-->22--(&)-->23
 * 1--(|)-->24--(|)-->25
 * 1--(blank)-->26--(blank(circle))-->26
 * 1--("|')-->27
 * 1--(\0)-->28
 *
 * accept state unit
 * all except 1
 */

static const int kMaxLexLength = 100;

// 'begin' and 'end' slides words
static int begin = 0, end = 0, buf_flag = 0;

int kType[] = {-1, ID, CONST, OP, OP, OP, OP, OP, OP, OP, OP, OP, OP, OP, SEP,
               OP, OP, OP, OP, UNUSED, UNUSED, OP, OP, OP, OP, UNUSED, CONST,
               UNUSED};

static char buffer[2][kBlockSize];

int isDigit(char c) {
    if (c >= '0' && c <= '9') {
        return 1;
    }
    return 0;
}

int isLetter(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        return 1;
    }
    return 0;
}

int isBlank(char c) {
    if (c == '\n' || c == '\t' || c == '\r' || c == ' ') {
        return 1;
    }
    return 0;
}

/**
 *
 * @param state
 * @param c
 * @return 0--temporary state
 *         1--reach accept state
 *         -1--error
 */
int move(int *state, char c) {
    if (*state == 1) {
        switch (c) {
            case '!':
            case '+':
            case '%':
            case '=':
                *state = 4;
                break;
            case '<':
                *state = 6;
                break;
            case '>':
                *state = 9;
                break;
            case '-':
                *state = 12;
                break;
            case '.':
            case ',':
            case ';':
            case '{':
            case '}':
            case '(':
            case ')':
            case '[':
            case ']':
            case '#':
            case ':':
                *state = 15;
                break;
            case '*':
                *state = 16;
                break;
            case '/':
                *state = 18;
                break;
            case '&':
                *state = 22;
                break;
            case '|':
                *state = 24;
                break;
            case '"':
            case '\'':
                *state = 27;
                break;
            case '\0':
                *state = 28;
                break;
            default:
                if (isLetter(c)) {
                    *state = 2;
                } else if (isDigit(c)) {
                    *state = 3;
                } else if (isBlank(c)) {
                    *state = 26;
                } else {
                    // unknown characters
                    return -1;
                }
        }
    } else if (*state == 2) {
        if (isLetter(c) || isDigit(c)) {
            *state = 2;
        } else {
            return 1;
        }
    } else if (*state == 3) {
        if (isDigit(c)) {
            *state = 3;
        } else {
            return 1;
        }
    } else if (*state == 4) {
        if (c == '=') {
            *state = 5;
        } else {
            return 1;
        }
    } else if (*state == 6) {
        switch (c) {
            case '<':
                *state = 7;
                break;
            case '=':
                *state = 8;
                break;
            default:
                return 1;
        }
    } else if (*state == 9) {
        switch (c) {
            case '>':
                *state = 10;
                break;
            case '=':
                *state = 11;
                break;
            default:
                return 1;
        }
    } else if (*state == 12) {
        switch (c) {
            case '>':
                *state = 13;
                break;
            case '=':
                *state = 14;
                break;
            default:
                return 1;
        }
    } else if (*state == 16) {
        switch (c) {
            case '*':
                *state = 16;
                break;
            case '=':
                *state = 17;
                break;
            default:
                return 1;
        }
    } else if (*state == 18) {
        switch (c) {
            case '=':
                *state = 19;
                break;
            case '*':
                *state = 20;
                break;
            case '/':
                *state = 21;
                break;
            default:
                return 1;
        }
    } else if (*state == 22) {
        if (c == '&') {
            *state = 23;
        } else {
            return 1;
        }
    } else if (*state == 24) {
        if (c == '|') {
            *state = 25;
        } else {
            return 1;
        }
    } else if (*state == 26) {
        if (isBlank(c)) {
            *state = 26;
        } else {
            return 1;
        }
    } else {
        return 1;
    }
    return 0;
}

void InitLex(int fd) {
    read(fd, buffer[0], kBlockSize);
}

void NextChar(int fd) {
    end++;
    if (end == kBlockSize) {
        buf_flag = !buf_flag;
        int r_count;
        if ((r_count = read(fd, buffer[buf_flag], kBlockSize)) < kBlockSize) {
            buffer[buf_flag][r_count] = '\0';
        }
        end = 0;
    }
    if (buffer[buf_flag][end] == '\n') {
        g_loc.line++;
        g_loc.column = 0;
    }
}

void NewErr(char *error) {
    ErrInfo err;
    err.errLoc.line = g_loc.line;
    err.errLoc.column = g_loc.column - (end - begin + kBlockSize) % kBlockSize;
    err.error = error;
    Enqueue(err_queue, err.next);
}

/**
 *
 * @param fd
 * @return A lex unit. If reaches eof, return 'NULL'.
 */
LexUnit *AnalyseLexical(int fd) {
    int state = 1, type;
    char lex_unit[kMaxLexLength];
    while (1) {
        int i = move(&state, buffer[buf_flag][end]);
        // unknown character
        if (i == -1) {
            NewErr("unknown character");
            NextChar(fd);
            begin = end;
            continue;
        }
        // reach accept state
        if (i == 1) {
            type = kType[state - 1];
            // skip '/*...*/' comment
            if (state == 20) {
                while (1) {
                    while (buffer[buf_flag][end] != '*') {
                        NextChar(fd);
                        if (buffer[buf_flag][end] == '\0') {
                            // cannot find right '*/'
                            NewErr("open comment");
                            return NULL;
                        }
                    }
                    NextChar(fd);
                    if (buffer[buf_flag][end] == '\0') {
                        // cannot find right '*/'
                        NewErr("open comment");
                        return NULL;
                    }
                    if (buffer[buf_flag][end] == '/') {
                        NextChar(fd);
                        break;
                    }
                }
            }
            // skip '//' comment
            if (state == 21) {
                while (buffer[buf_flag][end] != '\n' &&
                       buffer[buf_flag][end] != '\0') {
                    NextChar(fd);
                }
            }
            // string and character
            if (state == 27) {
                while (buffer[buf_flag][end] != buffer[buf_flag][begin]) {
                    if (buffer[buf_flag][end] == '\0') {
                        // not find right ' or " until eof
                        NewErr("lack of right quotation");
                        return NULL;
                    } else if (buffer[buf_flag][end] == '\\') {
                        NextChar(fd);
                        NextChar(fd);
                    } else {
                        NextChar(fd);
                    }
                }
                // copy string
                if (begin < end || begin == kBlockSize - 1) {
                    // string in a buffer area
                    strncpy(lex_unit,
                            buffer[buf_flag] + (begin + 1) % kBlockSize,
                            end - ((begin + 1) % kBlockSize));
                    lex_unit[end - begin - 1] = '\0';
                } else {
                    // string in two buffer areas
                    strncpy(lex_unit, buffer[!buf_flag] + begin + 1,
                            kBlockSize - begin - 1);
                    strncpy(lex_unit + kBlockSize - begin - 1,
                            buffer[buf_flag],
                            end);
                    lex_unit[kBlockSize - begin - 1 + end] = '\0';
                }
                NextChar(fd);
                begin = end;
                LexUnit *lex = malloc(sizeof(LexUnit));
                lex->type = type;
                lex->name = lex_unit;
                return lex;
            }
            // copy word to 'lex_unit'
            if (type != UNUSED) {
                if (begin < end) {
                    // lex unit in a buffer area
                    strncpy(lex_unit, buffer[buf_flag] + begin, end - begin);
                    lex_unit[end - begin] = '\0';
                } else {
                    // lex unit in two buffer areas
                    strncpy(lex_unit, buffer[!buf_flag] + begin,
                            kBlockSize - begin);
                    strncpy(lex_unit + kBlockSize - begin, buffer[buf_flag],
                            end);
                    lex_unit[kBlockSize - begin + end] = '\0';
                }
            }
            begin = end;
            if (type == UNUSED) {
                state = 1;
                continue;
            }
            LexUnit *lex = malloc(sizeof(LexUnit));
            lex->type = type;
            lex->name = lex_unit;
            return lex;
        }

        if (buffer[buf_flag][end] == '\0') {
            return NULL;
        }
        NextChar(fd);
    }
}