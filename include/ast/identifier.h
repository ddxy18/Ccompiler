//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_IDENTIFIER_H
#define CCOMPILER_IDENTIFIER_H

#include <list>
#include <memory>

#include "ast/expression.h"
#include "ast/translation_unit.h"

namespace CCompiler {
class CompoundStmt;

class Type;

class Identifier {
 private:
  std::unique_ptr<Type> type_;
  enum class Linkage {
    kExternal,
    kInternal,
    kNone
  };
  Linkage linkage_;
  // Scope of identifiers are implicitly included in the AST structure.
};

class Object : public Identifier, public Expr {
 private:
  enum class Storage {
    kStatic,
    kThread,
    kAutomatic,
    kAllocated
  };
  Storage storage_;
};

class Function : public Identifier, public ExternalDef {
  using ParamList = std::list<std::unique_ptr<Object>>;
 private:
  ParamList params_;
  std::unique_ptr<CompoundStmt> body_;
};
}

#endif // CCOMPILER_IDENTIFIER_H