//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_IDENTIFIER_H
#define CCOMPILER_IDENTIFIER_H

#include <list>

#include "ast/expression.h"

namespace CCompiler {
class CompoundStmt;

class Decl;

class Type;

class Identifier {
 public:
  enum class Linkage {
    kExternal,
    kInternal,
    kNone
  };

  Identifier(Type *type, Linkage linkage, std::string ident)
          : type_(type),
            linkage_(linkage),
            ident_(std::move(ident)) {}

  virtual ~Identifier() = default;

  [[nodiscard]] Linkage GetLinkage() const {
    return linkage_;
  }

 private:
  Type *type_;
  std::string ident_;
  Linkage linkage_;
};

// storage-class-specifier
enum {
  kTypedef = 0b1,
  kExtern = 0b10,
  kStatic = 0b100,
  k_Thread_local = 0b1000,
  kAuto = 0b10000,
  kRegister = 0b100000
};

class Object : public Identifier, public Expr {
  friend class ObjectDecl;

 public:
  enum class Storage {
    kStatic,
    kThread,
    kAutomatic,
    kAllocated  // aligned_alloc, calloc, malloc, and realloc
  };

  Object(Identifier *ident, int storage_spec)
          : Identifier(*ident),
            decl_(nullptr) {
    if (!(storage_spec & k_Thread_local) &&
        (ident->GetLinkage() == Linkage::kInternal ||
         ident->GetLinkage() == Linkage::kExternal ||
         storage_spec & kStatic)) {
      storage_ = Storage::kStatic;
    }
    if (storage_spec & k_Thread_local) {
      storage_ = Storage::kThread;
    }
    if (ident->GetLinkage() == Linkage::kNone && !(storage_spec & kStatic)) {
      storage_ = Storage::kAutomatic;
    }
  }

 private:
  Storage storage_;
  Decl *decl_;
};

class Function : public Identifier {
  using ParamList = std::list<Identifier *>;
 public:
  Function(Type *type, Linkage linkage, std::string ident, ParamList params)
          : Identifier(type, linkage, std::move(ident)),
            params_(std::move(params)),
            body_(nullptr) {}

  void BodyInit(CompoundStmt *body) {
    body_ = body;
  }

 private:
  ParamList params_;
  CompoundStmt *body_;
};
}

#endif // CCOMPILER_IDENTIFIER_H