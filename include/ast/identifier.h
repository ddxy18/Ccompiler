//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_IDENTIFIER_H
#define CCOMPILER_IDENTIFIER_H

#include <list>
#include <string>

#include "ast/expression.h"

namespace CCompiler {
class Type;

class CompoundStmt;

class Decl;

class Scope;

class Identifier {
 public:
  enum class Linkage {
    kExternal,
    kInternal,
    kNone
  };


  Identifier(Type *type, Linkage linkage, ::std::string ident)
          : type_(type),
            linkage_(linkage),
            ident_(::std::move(ident)) {}

  [[nodiscard]] Linkage GetLinkage() const {
    return linkage_;
  }

  [[nodiscard]] const ::std::string &GetIdent() const {
    return ident_;
  }

  bool operator==(const Identifier &rhs) const;

  bool operator!=(const Identifier &rhs) const {
    return !(rhs == *this);
  }

  virtual bool Equal(const Identifier *rhs) const {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(Identifier) && *this == *rhs;
  }

 private:
  Type *type_;
  ::std::string ident_;
  Linkage linkage_;
};

//!< storage-class-specifier
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
          : Expr(TokenType::kIdentifier),
            Identifier(*ident),
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

  bool IsIntConstant() override {
    return false;
  }

  int ToInt() override {
    exit(-1);
  }

  bool operator==(const Object &rhs) const;

  bool operator!=(const Object &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Identifier *rhs) const override {
    return typeid(*rhs) == typeid(Object) &&
           *this == *dynamic_cast<const Object *>(rhs);
  }

  bool Equal(const Expr *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(Object) &&
           *this == *dynamic_cast<const Object *>(rhs);
  }


 private:
  Storage storage_;
  Decl *decl_;
};

class Function : public Identifier, public Expr {
 public:
  using ParamList = std::list<Identifier *>;

  Function(Type *type, Linkage linkage, std::string ident,
           ParamList params,
           CompoundStmt *body = nullptr)
          : Expr(TokenType::kIdentifier),
            Identifier(type, linkage, std::move(ident)),
            params_(std::move(params)),
            body_(body) {}

  void BodyInit(CompoundStmt *body) {
    body_ = body;
  }

  void BodyInit(Function *func) {
    body_ = func->body_;
    func->body_ = nullptr;  // When we delete func, it will not influence body_.
  }

  bool IsIntConstant() override {
    return false;
  }

  int ToInt() override {
    exit(-1);
  }

  bool IsDefined() {
    return body_ != nullptr;
  }

  bool operator==(const Function &rhs) const;

  bool operator!=(const Function &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Identifier *rhs) const override {
    return typeid(*rhs) == typeid(Function) &&
           *this == *dynamic_cast<const Function *>(rhs);
  }

  bool Equal(const Expr *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(Function) &&
           *this == *dynamic_cast<const Function *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const;

 private:
  ParamList params_;
  CompoundStmt *body_;
};
}

#endif // CCOMPILER_IDENTIFIER_H