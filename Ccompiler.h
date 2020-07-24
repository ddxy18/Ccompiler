//
// Created by dxy on 2020/7/23.
//

#ifndef CCOMPILER_CCOMPILER_H
#define CCOMPILER_CCOMPILER_H

#define kBlockSize 4096

enum LexType {
    ID, KEYWORD, OP, SEP, CONST, UNUSED
};

typedef struct{
    enum LexType type;
    char *name;
}LexUnit;

void InitLex(int fd);
LexUnit *AnalyseLexical(int fd);

#endif //CCOMPILER_CCOMPILER_H