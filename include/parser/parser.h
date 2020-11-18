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
#include "ast/translation_unit.h"
#include "ast/type.h"
#include "lex/lexer.h"
#include "lex/token.h"

namespace CCompiler {
class Constant;

class Object;

class Decl;

class Initializer;

class CompoundStmt;

class Type;

class PointerType;

class Parser {
 public:
  explicit Parser(std::ifstream &source_file)
          : lexer_(source_file),
            trans_unit_(new TranslationUnit()),
            scope_(trans_unit_->GetScope()) {}

  /**
   * For testing.
   * @param source_string
   */
  explicit Parser(const std::string &source_string)
          : lexer_(source_string),
            trans_unit_(new TranslationUnit()),
            scope_(trans_unit_->GetScope()) {}

  TranslationUnit *Parse();

 private:
  /**
   * Parse function definitions and declarations with the file scope(including
   * function prototype).
   * @param trans_unit User should ensure its validity.
   */
  void ParseTranslateUnit();

  Type *ParseDeclSpec();

  static bool IsDeclSpec(const Token &token);

  /**
   * @param flag true--struct, false--union
   * @return
   */
  StructUnionType *ParseStructOrUnion(bool flag);

  EnumType *ParseEnum();

  /**
   * It only parses the declarations in the function definition. Declarations
   * with the file scope are dealt with in the ParseTranslateUnit(). It
   * parses several declarations with the same type split by ',' once.
   * @param scope the declarations belong to scope
   * @return at least have a element
   */
  std::list<Decl *> ParseDecl();

  Identifier *ParseDeclarator(Type *type);

  Expr *ParseConstExpr();

  PointerType *ParsePointer(Type *type);

  Initializer *ParseInitializer(Initializer::Element offset);

  /**
   *
   * @param scope User should construct the scope itself for the block if
   * needed.
   * @return
   */
  StmtList ParseCompoundStmt();

  StmtList ParseStmt();

  /**
   * Caller doesn't have to check the first several tokens to determine
   * whether it is a valid expression. This work will be done by the function.
   * @return
   */
  Expr *ParseExpr();

  Expr *ParseAssignExpr();

  Expr *ParseConditionalExpr();

  Expr *ParseLogicalOrExpr();

  Expr *ParseLogicalAndExpr();

  Expr *ParseBitOrExpr();

  Expr *ParseBitXorExpr();

  Expr *ParseBitAndExpr();

  Expr *ParseEqualityExpr();

  Expr *ParseRelationalExpr();

  Expr *ParseShiftExpr();

  Expr *ParseAdditiveExpr();

  Expr *ParseMultiplicativeExpr();

  Expr *ParseCastExpr();

  /**
   * Used in cast.
   * @return
   */
  Type *ParseTypeName();

  Expr *ParseUnaryExpr();

  Expr *ParsePostfixExpr();

  Expr *ParsePrimaryExpr();

  Constant *ParseIntConstExpr();

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
   * @tparam T
   * @param ParseOperand function to parse any one of the two operands
   * @param types expected operators
   * @return If we detect a required binary expression, it returns the
   * expression. Otherwise it will try to interpret the expression as it is
   * valid for ParseOperand().
   */
  template<class T>
  T ParseBinaryExpr(std::function<T()> ParseOperand,
                    const std::list<TokenType> &types);

  /**
   * Check whether the next token's type equals type. It will consume the
   * token after checking it.
   *
   * @param type
   * @return
   */
  Token Check(TokenType type);

  TranslationUnit *trans_unit_;

  Scope *scope_;

  Lexer lexer_;
};
}

#endif // CCOMPILER_PARSER_H