//
// Created by dxy on 2020/8/20.
//

#ifndef CCOMPILER_LEXER_H
#define CCOMPILER_LEXER_H

#include <fstream>
#include <sstream>
#include <vector>

namespace CCompiler {
class Token;

class Nfa;

using StrConstIt = std::string::const_iterator;

class Lexer {
  friend class Environment;

 public:
  explicit Lexer(std::ifstream &source_file) : line_(0), column_(0) {
    source_stream_ << source_file.rdbuf();
  }

  explicit Lexer(const std::string &source_string)
          : line_(0),
            column_(0),
            source_stream_(source_string) {}

  /**
   * Get and consume the next token.
   *
   * @return
   */
  Token Next();

  /**
   * Get but not consume the next token.
   *
   * @return
   */
  Token Peek();

  /**
   * Add a consumed token back to the Lexer.
   * @param token
   */
  void Rollback(const Token &token);

 private:
  /**
   * It gets a token from source_file_stream_. It can automatically
   * exclude some useless and invalid tokens.
   *
   * @return If no valid token remains, it returns an empty token.
   */
  Token NextToken();

  Token NextTokenInLine(StrConstIt &begin, StrConstIt &end);

  static Nfa nfa_;

  std::stringstream source_stream_;
  int line_;
  int column_;
  // store tokens that are got but not consumed immediately
  std::vector<Token> tokens_;
};
}

#endif // CCOMPILER_LEXER_H