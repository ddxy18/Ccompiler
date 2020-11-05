//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_PARSER_H
#define CCOMPILER_PARSER_H

#include <functional>
#include <list>
#include <map>
#include <set>

#include "ast/declaration.h"
#include "ast/type.h"
#include "lex/lexer.h"
#include "lex/token.h"

namespace CCompiler {
class Constant;

class TranslationUnit;

class Object;

class Decl;

class Initializer;

class CompoundStmt;

class Type;

class PointerType;

class Parser {
 public:
  explicit Parser(std::ifstream &source_file) : lexer_(source_file) {}

  TranslationUnit *Parse();

 private:
  void ParseTranslateUnit(TranslationUnit *trans_unit);

  Type *ParseDeclSpec();

  bool IsDeclSpec(Token token);

  /**
   * @param flag true--struct, false--union
   * @return
   */
  StructUnionType *ParseStructOrUnion(bool flag);

  EnumType *ParseEnum();

  std::list<Decl *> ParseDecl();

  Identifier *ParseDeclarator(Type *type);

  Constant *ParseConstExpr();

  PointerType *ParsePointer(Type *type);

  Initializer *ParseInitializer(Initializer::Element *offset);

  CompoundStmt *ParseCompoundStmt();

  Stmt *ParseStmt();

  /**
   * @tparam T
   * @param ParseElement
   * @param end Required token after the whole list. TokenType::kEmpty means
   * that the next token after the list doesn't have to be checked.
   * @param delim Split the element in the list. ',' is the default delimiter
   * . Notice that a valid element is guaranteed to be appear both before the
   * delim and after the delim.
   * @return
   */
  template<class T>
  std::list<T>
  ParseList(std::function<T(int)> ParseElement,
            TokenType end = TokenType::kEmpty,
            TokenType delim = TokenType::kComma);

  /**
   * Check whether the next token's type equals type. It will consume the
   * token after checking it.
   *
   * @param type
   * @return
   */
  Token Check(TokenType type);

  Lexer lexer_;
};
}

#endif // CCOMPILER_PARSER_H