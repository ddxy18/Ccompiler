//
// Created by dxy on 2020/9/28.
//

#include "parser/parser.h"

#include <functional>
#include <list>

#include "environment.h"
#include "ast/declaration.h"
#include "ast/translation_unit.h"

using namespace CCompiler;
using namespace std;

int storage_spec = 0;

TranslationUnit *Parser::Parse() {
  auto trans_unit = new TranslationUnit();

  while (!lexer_.Peek().Empty()) {
    lexer_.Rollback(lexer_.Next());
    ParseTranslateUnit(trans_unit);
  }

  return trans_unit;
}

void Parser::ParseTranslateUnit(TranslationUnit *trans_unit) {
  auto type = ParseDeclSpec();

  auto token = lexer_.Next();
  if (token.GetType() == TokenType::kSemicolon) {
    // type declaration
    trans_unit->AddExternalDef(new TypeDecl(type));
  } else {
    lexer_.Rollback(token);
    auto ident = ParseDeclarator(type);

    token = lexer_.Next();
    if (token.GetType() == TokenType::kAssign) {  // initializer for object
      trans_unit->AddExternalDef(new ObjectDecl(
              dynamic_cast<Object *>(ident),
              ParseInitializer(new Initializer::Element(0))));
    } else if (token.GetType() == TokenType::kLeftCurlyBracket) {
      // function definition
      dynamic_cast<Function *>(ident)->BodyInit(ParseCompoundStmt());
      trans_unit->AddExternalDef(new FuncDecl(dynamic_cast<Function *>(ident)));
      return;
    } else {  // uninitialized object
      trans_unit->AddExternalDef(
              new ObjectDecl(dynamic_cast<Object *>(ident), nullptr));
      lexer_.Rollback(token);
    }

    // object declaration
    token = lexer_.Next();
    if (token.GetType() == TokenType::kComma) {  // several object declarations
      for (auto &obj_decl:ParseList(
              function([this, type](int i) {
                  auto object = dynamic_cast<Object *>(ParseDeclarator(type));

                  auto token = lexer_.Next();
                  if (token.GetType() == TokenType::kAssign) {
                    return new ObjectDecl(
                            object,
                            ParseInitializer(new Initializer::Element(0)));
                  } else {
                    lexer_.Rollback(token);
                    return new ObjectDecl(object, nullptr);
                  }
              }),
              TokenType::kSemicolon)) {
        trans_unit->AddExternalDef(obj_decl);
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

CompoundStmt *Parser::ParseCompoundStmt() {
  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kRightCurlyBracket) {
    return new CompoundStmt();
  } else {
    auto *compound_stmt = new CompoundStmt();

    lexer_.Rollback(token);
    while (!(token = lexer_.Next()).Empty()) {
      if (token.GetType() == TokenType::kRightCurlyBracket) {
        return compound_stmt;
      } else {
        lexer_.Rollback(token);
        if (IsDeclSpec(token)) {
          for (auto &decl:ParseDecl()) {
            compound_stmt->AddStmt(decl);
          }
        } else {
          compound_stmt->AddStmt(ParseStmt());
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
  if (token.GetType() == TokenType::kLeftParenthesis) {  // function
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
    type = new ArrayType(type, ParseConstExpr()->GetConst().integer_);
    Check(TokenType::kRightBracket);
    while ((token = lexer_.Next()).GetType() == TokenType::kLeftBracket) {
      type = new ArrayType(type, ParseConstExpr()->GetConst().integer_);
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

Initializer *Parser::ParseInitializer(Initializer::Element *offset) {
  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kLeftCurlyBracket) {  // for {} initializer
    return new InitializerList(offset, ParseList(function([this](int offset) {
        auto *init = new InitializerList(new Initializer::Element(offset));

        auto *cur_init = init;
        InitializerList *last_init;
        Initializer::Element *designator_offset;
        Token token;
        bool designator = false;
        while (!(token = lexer_.Next()).Empty()) {
          if (token.GetType() == TokenType::kLeftBracket) {
            designator = true;
            designator_offset = new Initializer::Element(
                    ParseConstExpr()->GetConst().integer_);
            auto next_init = new InitializerList(designator_offset);
            cur_init->AddInit(next_init);
            last_init = cur_init;
            cur_init = next_init;
            Check(TokenType::kRightBracket);
          } else if (token.GetType() == TokenType::kDot) {
            designator = true;
            designator_offset = new Initializer::Element(
                    Check(TokenType::kIdentifier).GetToken());
            auto next_init = new InitializerList(designator_offset);
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
          return ParseInitializer(new Initializer::Element(offset));
        }
    }), TokenType::kRightCurlyBracket));
  } else {  // for single value initializer
    // TODO(dxy):
    // ParseAssignExpr();
//    return new BaseInitializer(offset, x);
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

Constant *Parser::ParseConstExpr() {
  // TODO(dxy):
  return nullptr;
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
                  token = lexer_.Next();
                  Check(TokenType::kNumber);
                  enumerator.value_ = stoi(token.GetToken());
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

Stmt *Parser::ParseStmt() {
  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kIdentifier) {
    auto ident = token;
    token = lexer_.Next();
    if (token.GetType() == TokenType::kColon) {  // labeled statement
      return new LabelStmt(new LabelStmt::Label(new Identifier(nullptr,
                                                               Identifier::Linkage::kNone,
                                                               ident.GetToken())),
                           ParseStmt());
    } else {  // expression statement started with ident
      lexer_.Rollback(token);
      lexer_.Rollback(ident);
      auto expr = ParseExpr();
      Check(TokenType::kSemicolon);
      return expr;
    }
    // labeled statement in switch statement
  } else if (token.GetType() == TokenType::kCase) {
    auto label = new LabelStmt::Label(ParseConstExpr()->GetConst().integer_);
    Check(TokenType::kColon);
    return new LabelStmt(label, ParseStmt());
  } else if (token.GetType() == TokenType::kDefault) {
    Check(TokenType::kColon);
    return new LabelStmt(new LabelStmt::Label(), ParseStmt());
  }
    // selection statement
  else if (token.GetType() == TokenType::kIf) {
    Check(TokenType::kLeftParenthesis);
    auto condition = ParseExpr();
    Check(TokenType::kRightParenthesis);
    auto if_stmt = ParseStmt();
    token = lexer_.Next();
    if (token.GetType() == TokenType::kElse) {
      return new IfStmt(condition, if_stmt, ParseStmt());
    } else {
      lexer_.Rollback(token);
      return new IfStmt(condition, if_stmt, nullptr);
    }
  } else if (token.GetType() == TokenType::kSwitch) {
    Check(TokenType::kLeftParenthesis);
    auto condition = ParseExpr();
    Check(TokenType::kRightParenthesis);
    return new SwitchStmt(condition, ParseStmt());
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
    return new WhileStmt(condition, ParseStmt());
  } else if (token.GetType() == TokenType::kDo) {
    auto stmt = ParseStmt();
    Check(TokenType::kWhile);
    Check(TokenType::kLeftParenthesis);
    auto condition = ParseExpr();
    Check(TokenType::kRightParenthesis);
    Check(TokenType::kSemicolon);
    return new DoWhileStmt(condition, stmt);
  } else if (token.GetType() == TokenType::kFor) {
    Check(TokenType::kLeftParenthesis);

    list<Stmt *> init;
    token = lexer_.Next();
    if (IsDeclSpec(token)) {  // declaration
      lexer_.Rollback(token);
      for (auto &decl:ParseDecl()) {
        init.push_back(decl);
      }
    } else if (token.GetType() != TokenType::kSemicolon) {  // expression
      lexer_.Rollback(token);
      init.push_back(ParseExpr());
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
    return new ForStmt(init, condition, after_loop, ParseStmt());
  }
    // jump statement
  else if (token.GetType() == TokenType::kGoto) {
    Check(TokenType::kSemicolon);
    auto ident = Check(TokenType::kIdentifier).GetToken();
    return new JumpStmt(JumpStmt::JumpType::kGoto, ident);
  } else if (token.GetType() == TokenType::kContinue) {
    Check(TokenType::kSemicolon);
    return new JumpStmt(JumpStmt::JumpType::kContinue);
  } else if (token.GetType() == TokenType::kBreak) {
    Check(TokenType::kSemicolon);
    return new JumpStmt(JumpStmt::JumpType::kBreak);
  } else if (token.GetType() == TokenType::kReturn) {
    token = lexer_.Next();
    if (token.GetType() == TokenType::kSemicolon) {
      return new ReturnStmt(nullptr);
    } else {
      lexer_.Rollback(token);
      auto return_value = ParseExpr();
      Check(TokenType::kSemicolon);
      return new ReturnStmt(return_value);
    }
  } else {
    return ParseExpr();
  }
}

std::list<Decl *> Parser::ParseDecl() {
  auto type = ParseDeclSpec();

  Token token = lexer_.Next();
  if (token.GetType() == TokenType::kSemicolon) {  // type
    return {new TypeDecl(type)};
  } else {
    lexer_.Rollback(token);
    return ParseList(function([this, type](int i) {
        auto ident = ParseDeclarator(type);

        auto token = lexer_.Next();
        if (token.GetType() == TokenType::kAssign) {  // initialized object
          return dynamic_cast<Decl *>(new ObjectDecl(
                  dynamic_cast<Object *>(ident),
                  ParseInitializer(new Initializer::Element(0))));
        } else {  // uninitialized object and function
          lexer_.Rollback(token);
          if (typeid(ident) == typeid(Object)) {
            return dynamic_cast<Decl *>(
                    new ObjectDecl(dynamic_cast<Object *>(ident), nullptr));
          } else {
            return dynamic_cast<Decl *>(
                    new FuncDecl(dynamic_cast<Function *>(ident)));
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
// TODO(dxy):
  return nullptr;
}