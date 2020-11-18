//
// Created by dxy on 2020/9/28.
//

#include "parser/parser.h"

#include <functional>
#include <list>

#include "environment.h"

using namespace CCompiler;
using namespace std;

int storage_spec = 0;

TranslationUnit *Parser::Parse() {
  while (!lexer_.Peek().Empty()) {
    lexer_.Rollback(lexer_.Next());
    ParseTranslateUnit();
  }

  return trans_unit_;
}

void Parser::ParseTranslateUnit() {
  auto type = ParseDeclSpec();

  auto token = lexer_.Next();
  if (token.GetType() == TokenType::kSemicolon) {
    // Struct, union, enum and typedef declaration. All other types will be
    // ignored.
    if (typeid(*type) == typeid(StructUnionType) ||
        typeid(*type) == typeid(EnumType) ||
        typeid(*type) == typeid(TypeDef)) {
      trans_unit_->AddExternalDef(
              new TypeDecl(dynamic_cast<TypeDeclType *>(type)));
    }
  } else {
    lexer_.Rollback(token);
    auto ident = ParseDeclarator(type);

    token = lexer_.Next();
    if (token.GetType() == TokenType::kAssign) {  // initializer for object
      trans_unit_->AddExternalDef(new ObjectDecl(
              dynamic_cast<Object *>(ident),
              ParseInitializer(0)));
    } else if (token.GetType() == TokenType::kLeftCurlyBracket) {
      // function definition
      auto func_scope =
              new Scope(Scope::ScopeType::kBlock, scope_);
      scope_ = func_scope;
      dynamic_cast<Function *>(ident)->BodyInit(
              new CompoundStmt(func_scope, ParseCompoundStmt()));
      scope_ = scope_->GetParent();
      trans_unit_->AddExternalDef(new FuncDecl(dynamic_cast<Function *>
                                               (ident)));
      return;
    } else if (token.GetType() == TokenType::kSemicolon) {
      // single uninitialized object and function prototype
      if (typeid(*ident) == typeid(Object)) {
        trans_unit_->AddExternalDef(
                new ObjectDecl(dynamic_cast<Object *>(ident), nullptr));
      } else {
        trans_unit_->AddExternalDef(
                new FuncDecl(dynamic_cast<Function *>(ident)));
      }
      return;
    } else {
      lexer_.Rollback(token);
    }

    token = lexer_.Next();
    if (token.GetType() == TokenType::kComma) {  // several object declarations
      for (auto &obj_decl:ParseList(
              function([this, type](int i) {
                  auto object = dynamic_cast<Object *>(ParseDeclarator(type));

                  auto token = lexer_.Next();
                  if (token.GetType() == TokenType::kAssign) {
                    return new ObjectDecl(object, ParseInitializer(0));
                  } else {
                    lexer_.Rollback(token);
                    return new ObjectDecl(object, nullptr);
                  }
              }),
              TokenType::kSemicolon)) {
        trans_unit_->AddExternalDef(obj_decl);
      }
    } else if (token.GetType() != TokenType::kSemicolon) {
      exit(-1);
    }
  }
  storage_spec = 0;
}

Type *Parser::ParseDeclSpec() {
  Token token;

  // struct, union and enum
  token = lexer_.Next();
  if (token.GetType() == TokenType::kStruct) {
    return ParseStructOrUnion(true);
  } else if (token.GetType() == TokenType::kUnion) {
    return ParseStructOrUnion(false);
  } else if (token.GetType() == TokenType::kEnum) {
    return ParseEnum();
  } else {
    lexer_.Rollback(token);
  }

  auto *type = new QualType();
  while (!(token = lexer_.Next()).Empty()) {
    // storage class specifier
    if (token.GetType() == TokenType::kExtern) {
      storage_spec |= kExtern;
    } else if (token.GetType() == TokenType::kStatic) {
      storage_spec |= kStatic;
    } else if (token.GetType() == TokenType::k_Thread_local) {
      storage_spec |= k_Thread_local;
    } else if (token.GetType() == TokenType::kAuto) {
      storage_spec |= kAuto;
    } else if (token.GetType() == TokenType::kRegister) {
      storage_spec |= kRegister;
    }
      // type specifier
    else if (token.GetType() == TokenType::kChar) {
      type->AddSpecifier(QualType::kChar);
    } else if (token.GetType() == TokenType::kShort) {
      type->AddSpecifier(QualType::kShort);
    } else if (token.GetType() == TokenType::kInt) {
      type->AddSpecifier(QualType::kInt);
    } else if (token.GetType() == TokenType::kLong) {
      type->AddSpecifier(QualType::kLong);
    } else if (token.GetType() == TokenType::kFloat) {
      type->AddSpecifier(QualType::kFloat);
    } else if (token.GetType() == TokenType::kDouble) {
      type->AddSpecifier(QualType::kDouble);
    } else if (token.GetType() == TokenType::kSigned) {
      type->AddSpecifier(QualType::kSigned);
    } else if (token.GetType() == TokenType::kUnsigned) {
      type->AddSpecifier(QualType::kUnsigned);
    } else if (token.GetType() == TokenType::k_Bool) {
      type->AddSpecifier(QualType::k_Bool);
    } else if (token.GetType() == TokenType::k_Complex) {
      type->AddSpecifier(QualType::k_Complex);
    } else if (token.GetType() == TokenType::kVoid) {
      type->AddSpecifier(QualType::kVoid);
    }
      // type qualifier
    else if (token.GetType() == TokenType::kConst) {
      type->AddQualifier(Qualifier::kConst);
    } else if (token.GetType() == TokenType::kRestrict) {
      type->AddQualifier(Qualifier::kRestrict);
    } else if (token.GetType() == TokenType::kVolatile) {
      type->AddQualifier(Qualifier::kVolatile);
    }
      // end of QualType
    else {
      lexer_.Rollback(token);
      break;
    }
  }
  return type;
}

StmtList Parser::ParseCompoundStmt() {
  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kRightCurlyBracket) {
    return StmtList();
  } else {
    StmtList stmt_list;

    lexer_.Rollback(token);
    while (!(token = lexer_.Next()).Empty()) {
      if (token.GetType() == TokenType::kRightCurlyBracket) {
        return stmt_list;
      } else {
        lexer_.Rollback(token);
        if (IsDeclSpec(token)) {
          for (auto &decl:ParseDecl()) {
            stmt_list.push_back(decl);
          }
        } else {
          auto new_stmt = ParseStmt();
          copy(new_stmt.begin(), new_stmt.end(),
               back_insert_iterator<list<Stmt *>>(stmt_list));
        }
      }
    }
    exit(-1);  // lack of }
  }
}

Identifier *Parser::ParseDeclarator(Type *type) {
  Identifier *ident;
  string name;

  // parse pointers
  auto token = lexer_.Next();
  if (token.GetType() == TokenType::kAsterisk) {
    type = ParsePointer(type);
  } else {
    lexer_.Rollback(token);
  }

  token = lexer_.Next();
  if (token.GetType() == TokenType::kIdentifier) {
    name = token.GetToken();
  } else {
    exit(-1);
  }

  token = lexer_.Next();
  if (token.GetType() == TokenType::kLeftParenthesis) {  // function prototype
    // parse params
    token = lexer_.Next();
    if (token.GetType() == TokenType::kRightParenthesis) {
      ident = new Function(type, Identifier::Linkage::kNone, name,
                           list<Identifier *>());
    } else {
      lexer_.Rollback(token);
      ident = new Function(
              type,
              Identifier::Linkage::kNone,
              name,
              ParseList(function([this](int i) {
                            auto type = ParseDeclSpec();
                            auto token = lexer_.Next();
                            if (token.GetType() == TokenType::kComma ||
                                token.GetType() ==
                                TokenType::kRightParenthesis) {
                              lexer_.Rollback(token);
                              return new Identifier(
                                      type, Identifier::Linkage::kNone, "");
                            }
                            return ParseDeclarator(type);
                        }),
                        TokenType::kRightParenthesis));
    }
  } else if (token.GetType() == TokenType::kLeftBracket) {  // array
    type = new ArrayType(type, ParseIntConstExpr()->ToInt());
    Check(TokenType::kRightBracket);
    while ((token = lexer_.Next()).GetType() == TokenType::kLeftBracket) {
      type = new ArrayType(type, ParseIntConstExpr()->ToInt());
      Check(TokenType::kRightBracket);
    }
    // TODO(dxy): determine linkage
    ident = new Object(new Identifier(type, Identifier::Linkage::kNone, name),
                       storage_spec);

    lexer_.Rollback(token);
  } else {
    // TODO(dxy): determine linkage
    ident = new Object(new Identifier(type, Identifier::Linkage::kNone, name),
                       storage_spec);

    lexer_.Rollback(token);
  }

  return ident;
}

Initializer *Parser::ParseInitializer(Initializer::Element offset) {
  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kLeftCurlyBracket) {  // for {} initializer
    return new InitializerList(
            std::move(offset), ParseList(function([this](int offset) {
                auto *init = new InitializerList(offset);

                auto *cur_init = init;
                InitializerList *last_init;
                Initializer::Element designator_offset;
                Token token;
                bool designator = false;
                while (!(token = lexer_.Next()).Empty()) {
                  if (token.GetType() == TokenType::kLeftBracket) {
                    designator = true;
                    designator_offset = ParseIntConstExpr()->ToInt();
                    auto next_init = new InitializerList(
                            get<int>(designator_offset));
                    cur_init->AddInit(next_init);
                    last_init = cur_init;
                    cur_init = next_init;
                    Check(TokenType::kRightBracket);
                  } else if (token.GetType() == TokenType::kDot) {
                    designator = true;
                    designator_offset =
                            Check(TokenType::kIdentifier).GetToken();
                    auto next_init = new InitializerList(
                            get<string>(designator_offset));
                    cur_init->AddInit(next_init);
                    last_init = cur_init;
                    cur_init = next_init;
                  } else {
                    lexer_.Rollback(token);
                    break;
                  }
                }
                if (designator) {
                  Check(TokenType::kAssign);
                  last_init->RemoveFirst();
                  last_init->AddInit(ParseInitializer(designator_offset));
                  return dynamic_cast<Initializer *>(init);
                } else {
                  return ParseInitializer(offset);
                }
            }), TokenType::kRightCurlyBracket));
  } else {  // for single value initializer
    // TODO(dxy):
    lexer_.Rollback(token);
    return new BaseInitializer(offset, ParseConditionalExpr());
  }
}

PointerType *Parser::ParsePointer(Type *type) {
  Type *derived = type;
  PointerType *ptr_type;
  Token token;

  token = Check(TokenType::kAsterisk);
  lexer_.Rollback(token);

  while ((token = lexer_.Next()).GetType() == TokenType::kAsterisk) {
    token = lexer_.Next();
    if (token.GetType() == TokenType::kConst) {
      ptr_type = new PointerType(derived, Qualifier::kConst);
      derived = ptr_type;
    } else if (token.GetType() == TokenType::k_Atomic) {
      ptr_type = new PointerType(derived, Qualifier::k_Atomic);
      derived = ptr_type;
    } else if (token.GetType() == TokenType::kVolatile) {
      ptr_type = new PointerType(derived, Qualifier::kVolatile);
      derived = ptr_type;
    } else if (token.GetType() == TokenType::kRestrict) {
      ptr_type = new PointerType(derived, Qualifier::kRestrict);
      derived = ptr_type;
    } else {
      ptr_type = new PointerType(derived, Qualifier::kEmpty);
      derived = ptr_type;
      lexer_.Rollback(token);
    }
  }
  lexer_.Rollback(token);

  return ptr_type;
}

Token Parser::Check(TokenType type) {
  auto token = lexer_.Next();

  if (token.GetType() == type) {
    return token;
  }

  // The actual type is different from the required type, which means that
  // the source file obeys the C language syntax. Now we don't provide any
  // error messages, so we just call exit() to stop the compiler.
  exit(-1);
}

Expr *Parser::ParseConstExpr() {
  // TODO(dxy):
  auto expr = ParseConditionalExpr();
}

StructUnionType *Parser::ParseStructOrUnion(bool flag) {
  StructUnionType *type;

  auto token = lexer_.Next();
  if (token.GetType() == TokenType::kIdentifier) {
    type = new StructUnionType(flag, token.GetToken());
    token = lexer_.Next();
    if (token.GetType() != TokenType::kLeftCurlyBracket) {
      lexer_.Rollback(token);
      return type;
    }
  } else if (token.GetType() == TokenType::kLeftCurlyBracket) {
    type = new StructUnionType(flag);
  } else {
    exit(-1);
  }

  // parse struct-declaration-list
  while ((token = lexer_.Next()).GetType() != TokenType::kRightCurlyBracket) {
    lexer_.Rollback(token);
    for (auto &element: ParseList(
            function(
                    [this](int i) { return ParseDeclarator(ParseDeclSpec()); }),
            TokenType::kSemicolon)) {
      type->AddMember(element);
    }
  }

  return type;
}

EnumType *Parser::ParseEnum() {
  Token token;
  string ident;
  EnumType *enum_type;

  token = lexer_.Next();
  if (token.GetType() == TokenType::kIdentifier) {
    enum_type = new EnumType(token.GetToken());

    token = lexer_.Next();
    if (token.GetType() != TokenType::kLeftCurlyBracket) {
      lexer_.Rollback(token);
      return enum_type;
    }
  } else if (token.GetType() == TokenType::kLeftCurlyBracket) {
    enum_type = new EnumType("");
  } else {
    exit(-1);
  }

  // parse enumerator-list
  for (auto &enumerator:ParseList(
          function([this](int i) {
              EnumType::Enumerator enumerator;
              auto token = lexer_.Next();
              if (token.GetType() == TokenType::kIdentifier) {
                enumerator.ident_ = token.GetToken();
                token = lexer_.Next();
                if (token.GetType() == TokenType::kAssign) {
                  enumerator.value_ = ParseIntConstExpr()->ToInt();
                } else {
                  lexer_.Rollback(token);
                }
              } else {
                exit(-1);
              }
              return enumerator;
          }),
          TokenType::kRightCurlyBracket)) {
    enum_type->AddEnumerator(enumerator);
  }

  return enum_type;
}

template<class T>
list<T>
Parser::ParseList(function<T(int)> ParseElement,
                  TokenType end, TokenType delim) {
  Token token;
  list<T> elements;
  int i = 0;

  elements.push_back(ParseElement(i++));
  while ((token = lexer_.Next()).GetType() == delim) {
    elements.push_back(ParseElement(i++));
  }
  lexer_.Rollback(token);

  if (end != TokenType::kEmpty) {
    Check(end);
  }

  return elements;
}

StmtList Parser::ParseStmt() {
  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kIdentifier) {
    auto ident = token;
    token = lexer_.Next();
    if (token.GetType() == TokenType::kColon) {  // identifier labeled statement
      return {new LabelStmt(LabelStmt::Label(new Identifier(nullptr,
                                                            Identifier::Linkage::kNone,
                                                            ident.GetToken())),
                            ParseStmt())};
    } else {  // expression statement started with ident
      lexer_.Rollback(token);
      lexer_.Rollback(ident);
      auto *expr = ParseExpr();
      Check(TokenType::kSemicolon);
      return {new ExprStmt(expr)};
    }
    // labeled statement in switch statement
  } else if (token.GetType() == TokenType::kCase) {
    auto label =
            LabelStmt::Label(ParseIntConstExpr()->ToInt());
    Check(TokenType::kColon);
    return {new LabelStmt(label, ParseStmt())};
  } else if (token.GetType() == TokenType::kDefault) {
    Check(TokenType::kColon);
    return {new LabelStmt(LabelStmt::Label(), ParseStmt())};
  }
    // selection statement
  else if (token.GetType() == TokenType::kIf) {
    Check(TokenType::kLeftParenthesis);
    auto condition = ParseExpr();
    Check(TokenType::kRightParenthesis);
    auto if_scope = new Scope(Scope::ScopeType::kBlock, scope_);
    scope_ = if_scope;
    auto if_stmt = ParseStmt();
    scope_ = scope_->GetParent();
    token = lexer_.Next();
    if (token.GetType() == TokenType::kElse) {
      auto else_scope = new Scope(Scope::ScopeType::kBlock, scope_);
      scope_ = else_scope;
      auto else_stmt = ParseStmt();
      scope_ = scope_->GetParent();
      return {new IfStmt(if_scope, if_stmt,
                         condition,
                         new CompoundStmt(else_scope, else_stmt))};
    } else {
      lexer_.Rollback(token);
      return {new IfStmt(if_scope, if_stmt, condition)};
    }
  } else if (token.GetType() == TokenType::kSwitch) {
    Check(TokenType::kLeftParenthesis);
    auto condition = ParseExpr();
    Check(TokenType::kRightParenthesis);
    auto switch_scope = new Scope(Scope::ScopeType::kBlock, scope_);
    scope_ = switch_scope;
    auto switch_stmt = new SwitchStmt(switch_scope, condition, ParseStmt());
    scope_ = scope_->GetParent();
    return {switch_stmt};
  }
    // compound statement
  else if (token.GetType() == TokenType::kLeftCurlyBracket) {
    return ParseCompoundStmt();
  }
    // iteration statement
  else if (token.GetType() == TokenType::kWhile) {
    Check(TokenType::kLeftParenthesis);
    auto condition = ParseExpr();
    Check(TokenType::kRightParenthesis);
    auto while_scope = new Scope(Scope::ScopeType::kBlock, scope_);
    scope_ = while_scope;
    auto while_stmt = new WhileStmt(while_scope, ParseStmt(),
                                    condition,
                                    true);
    while_scope = while_scope->GetParent();
    return {while_stmt};
  } else if (token.GetType() == TokenType::kDo) {
    auto while_scope = new Scope(Scope::ScopeType::kBlock, scope_);
    scope_ = while_scope;
    auto stmt = ParseStmt();
    Check(TokenType::kWhile);
    Check(TokenType::kLeftParenthesis);
    auto condition = ParseExpr();
    Check(TokenType::kRightParenthesis);
    Check(TokenType::kSemicolon);
    scope_ = scope_->GetParent();
    return {new WhileStmt(while_scope, stmt, condition, false)};
  } else if (token.GetType() == TokenType::kFor) {
    auto for_scope = new Scope(Scope::ScopeType::kBlock, scope_);
    scope_ = for_scope;

    Check(TokenType::kLeftParenthesis);

    StmtList init;
    token = lexer_.Next();
    if (IsDeclSpec(token)) {  // declaration
      lexer_.Rollback(token);
      for (auto &decl:ParseDecl()) {
        init.push_back(decl);
      }
    } else if (token.GetType() != TokenType::kSemicolon) {  // expression
      lexer_.Rollback(token);
      init.push_back(new ExprStmt(ParseExpr()));
      Check(TokenType::kSemicolon);
    }

    Expr *condition = nullptr;
    token = lexer_.Next();
    if (token.GetType() != TokenType::kSemicolon) {
      lexer_.Rollback(token);
      condition = ParseExpr();
      Check(TokenType::kSemicolon);
    }

    Expr *after_loop = nullptr;
    token = lexer_.Next();
    if (token.GetType() != TokenType::kRightParenthesis) {
      lexer_.Rollback(token);
      after_loop = ParseExpr();
    } else {
      lexer_.Rollback(token);
    }

    Check(TokenType::kRightParenthesis);
    auto for_stmt = new ForStmt(for_scope, ParseStmt(),
                                init,
                                condition,
                                after_loop);
    scope_ = scope_->GetParent();
    return {for_stmt};
  }
    // jump statement
  else if (token.GetType() == TokenType::kGoto) {
    Check(TokenType::kSemicolon);
    auto ident = Check(TokenType::kIdentifier).GetToken();
    return {new JumpStmt(JumpStmt::JumpType::kGoto, ident)};
  } else if (token.GetType() == TokenType::kContinue) {
    Check(TokenType::kSemicolon);
    return {new JumpStmt(JumpStmt::JumpType::kContinue)};
  } else if (token.GetType() == TokenType::kBreak) {
    Check(TokenType::kSemicolon);
    return {new JumpStmt(JumpStmt::JumpType::kBreak)};
  } else if (token.GetType() == TokenType::kReturn) {
    token = lexer_.Next();
    if (token.GetType() == TokenType::kSemicolon) {
      return {new ReturnStmt(nullptr)};
    } else {
      lexer_.Rollback(token);
      auto return_value = ParseExpr();
      Check(TokenType::kSemicolon);
      return {new ReturnStmt(return_value)};
    }
  } else {
    return {new ExprStmt(ParseExpr())};
  }
}

std::list<Decl *> Parser::ParseDecl() {
  auto type = ParseDeclSpec();

  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kSemicolon) {  // type
    // Struct, union, enum and typedef declaration. All other types will be
    // ignored.
    if (typeid(*type) == typeid(StructUnionType) ||
        typeid(*type) == typeid(EnumType) ||
        typeid(*type) == typeid(TypeDef)) {
      auto type_decl = new TypeDecl(dynamic_cast<TypeDeclType *>(type));
      scope_->AddIdent(type_decl);
      return {type_decl};
    }
    return {};
  } else {
    lexer_.Rollback(token);
    return ParseList(function([this, type](int i) {
        auto ident = ParseDeclarator(type);

        auto token = lexer_.Next();
        if (token.GetType() == TokenType::kAssign) {  // initialized object
          auto obj_decl = new ObjectDecl(dynamic_cast<Object *>(ident),
                                         ParseInitializer(0));
          scope_->AddIdent(obj_decl);
          return dynamic_cast<Decl *>(obj_decl);
        } else {  // uninitialized object
          lexer_.Rollback(token);
          if (typeid(*ident) == typeid(Object)) {
            auto obj_decl = new ObjectDecl(dynamic_cast<Object *>(ident),
                                           nullptr);
            scope_->AddIdent(obj_decl);
            return dynamic_cast<Decl *>(obj_decl);
          } else {
            // Since a function cannot be defined within another function, we
            // don't have to consider functions here.
            exit(-1);
          }
        }
    }), TokenType::kSemicolon);
  }
}

bool Parser::IsDeclSpec(const Token &token) {
  if (token.GetType() == TokenType::kStruct ||
      token.GetType() == TokenType::kUnion ||
      token.GetType() == TokenType::kEnum ||
      token.GetType() == TokenType::kExtern ||
      token.GetType() == TokenType::kStatic ||
      token.GetType() == TokenType::k_Thread_local ||
      token.GetType() == TokenType::kAuto ||
      token.GetType() == TokenType::kRegister ||
      token.GetType() == TokenType::kChar ||
      token.GetType() == TokenType::kShort ||
      token.GetType() == TokenType::kInt ||
      token.GetType() == TokenType::kLong ||
      token.GetType() == TokenType::kFloat ||
      token.GetType() == TokenType::kDouble ||
      token.GetType() == TokenType::kSigned ||
      token.GetType() == TokenType::kUnsigned ||
      token.GetType() == TokenType::k_Bool ||
      token.GetType() == TokenType::k_Complex ||
      token.GetType() == TokenType::kVoid ||
      token.GetType() == TokenType::kConst ||
      token.GetType() == TokenType::kRestrict ||
      token.GetType() == TokenType::kVolatile) {
    return true;
  }
  return false;
}

Expr *Parser::ParseExpr() {
  Expr *expr;

  // expressions split by ','
  auto expr_list = ParseList(function([this](int i) {
      return ParseAssignExpr();
  }));
  // build the expression tree
  expr = *expr_list.cbegin();
  expr_list.erase(expr_list.cbegin());
  for (auto &e:expr_list) {
    expr = new BinaryExpr(TokenType::kComma, expr, e);
  }

  return expr;
}

Expr *Parser::ParseAssignExpr() {
  return ParseBinaryExpr(function([this]() { return ParseConditionalExpr(); }),
                         {TokenType::kAssign,
                          TokenType::kMultiAssign,
                          TokenType::kDivideAssign,
                          TokenType::kModuloAssign,
                          TokenType::kPlusAssign,
                          TokenType::kMinusAssign,
                          TokenType::kLeftShiftAssign,
                          TokenType::kRightShiftAssign,
                          TokenType::kAndAssign,
                          TokenType::kXorAssign,
                          TokenType::kOrAssign});
}

Expr *Parser::ParseConditionalExpr() {
  auto expr = ParseLogicalOrExpr();

  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kQuestion) {
    auto operand2 = ParseExpr();
    Check(TokenType::kColon);
    auto operand3 = ParseConditionalExpr();
    return new ConditionalExpr(TokenType::kQuestion, expr, operand2, operand3);
  } else {
    lexer_.Rollback(token);
    return expr;
  }
}

Expr *Parser::ParseLogicalOrExpr() {
  return ParseBinaryExpr(function([this]() { return ParseLogicalAndExpr(); }),
                         {TokenType::kLogicalOr});
}

Expr *Parser::ParseLogicalAndExpr() {
  return ParseBinaryExpr(function([this]() { return ParseBitOrExpr(); }),
                         {TokenType::kLogicalAnd});
}

Expr *Parser::ParseBitOrExpr() {
  return ParseBinaryExpr(function([this]() { return ParseBitXorExpr(); }),
                         {TokenType::kBitOr});
}

Expr *Parser::ParseBitXorExpr() {
  return ParseBinaryExpr(function([this]() { return ParseBitAndExpr(); }),
                         {TokenType::kBitAnd});
}

Expr *Parser::ParseBitAndExpr() {
  return ParseBinaryExpr(function([this]() { return ParseEqualityExpr(); }),
                         {TokenType::kBitAnd});
}

Expr *Parser::ParseEqualityExpr() {
  return ParseBinaryExpr(function([this]() { return ParseRelationalExpr(); }),
                         {TokenType::kEqual,
                          TokenType::kNotEqual});
}

Expr *Parser::ParseRelationalExpr() {
  return ParseBinaryExpr(function([this]() { return ParseShiftExpr(); }),
                         {TokenType::kLess,
                          TokenType::kMore,
                          TokenType::kLessEqual,
                          TokenType::kMoreEqual});
}

Expr *Parser::ParseShiftExpr() {
  return ParseBinaryExpr(function([this]() { return ParseAdditiveExpr(); }),
                         {TokenType::kLeftShift,
                          TokenType::kRightShift,
                          TokenType::kLessEqual,
                          TokenType::kMoreEqual});
}

Expr *Parser::ParseAdditiveExpr() {
  return ParseBinaryExpr(function([this]() {
                             return ParseMultiplicativeExpr();
                         }),
                         {TokenType::kPlus,
                          TokenType::kMinus});
}

Expr *Parser::ParseMultiplicativeExpr() {
  return ParseBinaryExpr(function([this]() {
                             return ParseCastExpr();
                         }),
                         {TokenType::kAsterisk,
                          TokenType::kDivide,
                          TokenType::kModulo});
}

template<class T>
T Parser::ParseBinaryExpr(function<T()> ParseOperand,
                          const list<TokenType> &types) {
  auto l_operand = ParseOperand();

  auto token = lexer_.Next();
  for (auto &type:types) {
    if (token.GetType() == type) {
      return new BinaryExpr(type, l_operand, ParseOperand());
    }
  }
  lexer_.Rollback(token);

  return l_operand;
}

Expr *Parser::ParseCastExpr() {
  // TODO(dxy):
  /*Token token=lexer_.Next();
  if (token.GetType()==TokenType::kLeftParenthesis){
    token=lexer_.Next();
    if (IsDeclSpec(token)){
      lexer_.Rollback(token);
      ParseDeclSpec();
    } else if (token.GetType()==TokenType::kIdentifier){

    }
  } else{
    lexer_.Rollback(token);
    return ParseUnaryExpr();
  }*/
  return ParseUnaryExpr();
}

Expr *Parser::ParseUnaryExpr() {
  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kIncrement ||
      token.GetType() == TokenType::kDecrement) {
    return new UnaryExpr(token.GetType(), ParseUnaryExpr());
  } else if (token.GetType() == TokenType::kSizeof) {
    token = lexer_.Next();
    if (token.GetType() == TokenType::kLeftParenthesis) {
      // TODO(dxy):
//      ParseTypeName();
    } else {
      lexer_.Rollback(token);
      return new UnaryExpr(TokenType::kSizeof, ParseUnaryExpr());
    }
  } else if (token.GetType() == TokenType::k_Alignof) {
    // TODO(dxy):
  } else if (token.GetType() == TokenType::kBitAnd ||
             token.GetType() == TokenType::kAsterisk ||
             token.GetType() == TokenType::kPlus ||
             token.GetType() == TokenType::kMinus ||
             token.GetType() == TokenType::kTilde ||
             token.GetType() == TokenType::kLogicalNot) {
    return new UnaryExpr(token.GetType(), ParseCastExpr());
  } else {
    lexer_.Rollback(token);
    return ParsePostfixExpr();
  }
}

Expr *Parser::ParsePostfixExpr() {
  // TODO(dxy): deal with ( type-name ) { initializer-list }
  //                      ( type-name ) { initializer-list , }
  Expr *expr = ParsePrimaryExpr();
  Function *func = nullptr;
  Object *obj = nullptr;
  if (typeid(*expr) == typeid(Object)) {
    obj = scope_->GetObject(dynamic_cast<Object *>(expr)->GetIdent());
    func = scope_->GetFunc(dynamic_cast<Object *>(expr)->GetIdent());
    delete expr;
    expr = obj;
  }

  Token token;
  int i = 0;
  while (!(token = lexer_.Next()).Empty()) {
    if (token.GetType() == TokenType::kLeftBracket) {  // array
      // check whether the object has been declared
      if (i == 0) {
        if (obj != nullptr) {
          expr = obj;
        } else {
          exit(-1);
        }
      }

      auto index = ParseExpr();
      Check(TokenType::kRightBracket);
      expr = new ArrayExpr(expr, index);
    } else if (token.GetType() == TokenType::kLeftParenthesis) {  // func call
      // Func call is like xxx(params). The only possible condition is that
      // expr contains a func_list and params are parsed later. So we should
      // parse the whole func call at the first iteration of the while
      // statement.
      if (i != 0 || func == nullptr) {
        exit(-1);
      }

      // parse parameters
      token = lexer_.Next();
      if (token.GetType() == TokenType::kRightParenthesis) {
        return new FuncCall(func, FuncCall::ParamList());
      } else {
        lexer_.Rollback(token);
        auto params = ParseList(function([this](int i) {
                                    return ParseAssignExpr();
                                }),
                                TokenType::kRightParenthesis);
        Check(TokenType::kRightParenthesis);
        return new FuncCall(func, params);
      }
    } else if (token.GetType() == TokenType::kDot ||
               token.GetType() == TokenType::kArrow) {  // dereference
      // check whether the object has been declared
      if (i == 0) {
        if (obj != nullptr) {
          expr = obj;
        } else {
          exit(-1);
        }
      }

      auto ident = Check(TokenType::kIdentifier);
      // TODO(dxy): determine the type of the ident
      expr = new BinaryExpr(token.GetType(),
                            expr,
                            new Object(new Identifier(nullptr,
                                                      Identifier::Linkage::kNone,
                                                      ident.GetToken()),
                                       0));
    } else if (token.GetType() == TokenType::kIncrement ||
               token.GetType() == TokenType::kDecrement) {
      // check whether the object has been declared
      if (i == 0) {
        if (obj != nullptr) {
          expr = obj;
        } else {
          exit(-1);
        }
      }

      expr = new UnaryExpr(token.GetType(), expr, true);
    } else {
      lexer_.Rollback(token);
      break;
    }

    i++;
  }
  return expr;
}

Expr *Parser::ParsePrimaryExpr() {
  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kIdentifier) {  // object or func call
    // We don't distinguish between object and func call here. It uses a
    // temporary Object to wrap the token so we can use the same return type.
    return new Object(new Identifier(nullptr,
                                     Identifier::Linkage::kNone,
                                     token.GetToken()),
                      0);
  } else if (token.GetType() == TokenType::kNumber) {
    // Since all floating constants must contain '.' and integer constants
    // contain no '.', we use '.' to distinguish between integer constants
    // and floating constants.
    if (token.GetToken().find('.') == string::npos) {
      return new Constant(stoi(token.GetToken()));
    }
    return new Constant(stof(token.GetToken()));
  } else if (token.GetType() == TokenType::kCharacter) {
    return new Constant(token.GetToken()[0]);
  } else if (token.GetType() == TokenType::kString) {
    return new Constant(
            string(token.GetToken().begin() + 1, token.GetToken().end() - 1));
  } else if (token.GetType() == TokenType::kLeftParenthesis) {
    auto expr = ParseExpr();
    Check(TokenType::kRightParenthesis);
    return expr;
  } else if (token.GetType() == TokenType::k_Generic) {
    // TODO(dxy): generic selection
  } else {
    exit(-1);
  }
}

Type *Parser::ParseTypeName() {
  // TODO(dxy):
  return nullptr;
}

Constant *Parser::ParseIntConstExpr() {
  auto *expr = ParseConditionalExpr();
  if (expr->IsIntConstant()) {
    return new Constant(expr->ToInt());
  }
  exit(-1);
}
