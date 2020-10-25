//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_IDENTIFIER_H
#define CCOMPILER_IDENTIFIER_H

#include <list>
#include <memory>

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

class Object : public Identifier {
 private:
  enum class Storage {
    kStatic,
    kThread,
    kAutomatic,
    kAllocated
  };
  Storage storage_;
};

class Function : public Identifier {
  using ParamList = std::list<std::unique_ptr<Object>>;
 private:
  ParamList param_list_;
  std::unique_ptr<CompoundStmt> body_;
};
}

#endif // CCOMPILER_IDENTIFIER_H