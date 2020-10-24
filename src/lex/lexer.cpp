//
// Created by dxy on 2020/8/20.
//

#include "lex/lexer.h"

#include "environment.h"

using namespace CCompiler;
using namespace std;

Nfa Lexer::nfa_ = Nfa(map<string, int>());

Token Lexer::Next() {
  if (tokens_.empty()) {
    return NextToken();
  } else {
    auto token = tokens_.front();
    tokens_.pop();
    return token;
  }
}

Token Lexer::Peek() {
  auto token = NextToken();
  if (!token.Empty()) {
    tokens_.push(token);
  }
  return token;
}

Token Lexer::NextToken() {
  static string line;
  static auto begin = line.cbegin(), end = line.cend();
  static bool line_flag = true;  // whether to read a new word from the file

  while (true) {
    if (line_flag) {
      while (getline(source_file_stream_, line)) {
        line_++;
        column_ = 0;
        line_flag = false;
        begin = line.cbegin(), end = line.cend();
        Token token = NextTokenInLine(begin, end);
        if (!token.Empty()) {
          return token;
        }
      }

      return Token();  // reach to the end of the file
    } else {
      Token token = NextTokenInLine(begin, end);
      if (token.Empty()) {  // reach to the end of a line
        line_flag = true;
        continue;
      }
      return token;
    }
  }
}

Token Lexer::NextTokenInLine(StrConstIt &begin, StrConstIt &end) {
  static bool comment_flag = false;  // used to skip /**/ comments

  while (begin != end) {
    auto pair = nfa_.NextMatch(begin, end);
    if (pair == nullptr || begin == pair->second) {  // invalid token
      // ignore the current character to find the next valid token
      begin++;
      continue;
    }

    Token token(string{begin, pair->second}, pair->first);
    begin = pair->second;
    if (comment_flag) {
      if (token.GetToken() == "*/") {
        comment_flag = false;
      }
      column_ += token.GetToken().size();
      continue;
    }

    if (token.GetType() == Environment::IntSymbol("string") ||
        token.GetType() == Environment::IntSymbol("character")) {
      auto tmp = begin - 1;
      while (begin != end) {
        if (*begin == token.GetToken()[0] && *(begin - 1) != '\\') {
          begin++;
          token.SetToken(string(tmp, begin));
          token.SetLine(line_);
          token.SetColumn(column_);
          column_ += begin - tmp;
          return token;
        }
        begin++;
      }
    } else if (token.GetType() == Environment::IntSymbol("comment")) {
      if (token.GetToken() == "//") {  // skip the line
        begin = end;
      } else if (token.GetToken() == "/*") {
        comment_flag = true;
        column_ += token.GetToken().size();
      }
    } else if (token.GetType() == Environment::IntSymbol("delim")) {
      column_ += token.GetToken().size();
    } else {
      token.SetLine(line_);
      token.SetColumn(column_);
      column_ += token.GetToken().size();
      return token;
    }
  }
  return Token();
}