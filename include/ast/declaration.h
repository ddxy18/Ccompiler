//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_DECLARATION_H
#define CCOMPILER_DECLARATION_H

#include <list>
#include <string>
#include <utility>
#include <variant>

#include "ast/statement.h"

namespace CCompiler {
class DerivedType;

class Expr;

class Function;

class Initializer;

class Object;

class Parser;

class TypeDeclType;

class Stmt;

class Decl : public Stmt {
 public:
  virtual ::std::string GetIdent() = 0;

  bool operator==(const Decl &rhs) const {
    return true;
  }

  bool operator!=(const Decl &rhs) const {
    return !(rhs == *this);
  }

  virtual bool Equal(const Decl *rhs) const = 0;

  [[nodiscard]] Scope *GetOwnedScope() const override = 0;
};

class Initializer {
  friend bool operator==(const Initializer &lhs, const Initializer &rhs);

 public:
  /**
   * int-- relative location of an element in an array
   * std::string--a member in a struct or an union
   */
  using Element = std::variant<int, std::string>;

  explicit Initializer(Element offset) : offset_(std::move(offset)) {}

  explicit Initializer(int offset) : offset_(offset) {}

  explicit Initializer(std::string member) : offset_(member) {}

  bool operator!=(const Initializer &rhs) const {
    return !(rhs == *this);
  }

  virtual bool Equal(const Initializer *rhs) const {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(Initializer) && *this == *rhs;
  }


 private:
  Element offset_;
};

bool operator==(const Initializer &lhs, const Initializer &rhs);

/**
 * single value initializer
 */
class BaseInitializer : public Initializer {
 public:
  BaseInitializer(const Element &offset, Expr *init_value)
          : Initializer(offset),
            init_value_(init_value) {}

  BaseInitializer(int offset, Expr *init_value)
          : Initializer(offset),
            init_value_(init_value) {}

  BaseInitializer(std::string member, Expr *init_value)
          : Initializer(std::move(member)),
            init_value_(init_value) {}

  bool operator==(const BaseInitializer &rhs) const;

  bool operator!=(const BaseInitializer &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Initializer *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(BaseInitializer) &&
           *this == *dynamic_cast<const BaseInitializer *>(rhs);
  }

 private:
  Expr *init_value_;
};

/**
 * {} initializer
 */
class InitializerList : public Initializer {
 public:
  explicit InitializerList(Element offset,
                           std::list<Initializer *> init_list = std::list<Initializer *>())
          : Initializer(std::move(offset)),
            init_list_(std::move(init_list)) {}

  explicit InitializerList(int offset,
                           std::list<Initializer *> init_list = std::list<Initializer *>())
          : Initializer(offset),
            init_list_(std::move(init_list)) {}

  explicit InitializerList(std::string member,
                           std::list<Initializer *> init_list = std::list<Initializer *>())
          : Initializer(std::move(member)),
            init_list_(std::move(init_list)) {}

  void AddInit(Initializer *init) {
    init_list_.push_back(init);
  }

  /**
   * @brief ParseInitializer(Offset *) will use this function.
   */
  void RemoveFirst() {
    init_list_.erase(init_list_.cbegin());
  }

  bool operator==(const InitializerList &rhs) const;

  bool operator!=(const InitializerList &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Initializer *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(InitializerList) &&
           *this == *dynamic_cast<const InitializerList *>(rhs);
  }

 private:
  std::list<Initializer *> init_list_;
};

/**
 * @brief declarations for objects
 *
 * It contains information about a initializer if it is given explicitly in
 * the code. Decl is inherited from Stmt so we can combine the declarations
 * and statements in a block, which simplifies the structure for CompoundStmt.
 */
class ObjectDecl : public Decl {
 public:
  ObjectDecl(Object *object, Initializer *initializer);

  [[nodiscard]] Object *GetObject() const {
    return object_;
  }

  std::string GetIdent() override;

  bool operator==(const ObjectDecl &rhs) const;

  bool operator!=(const ObjectDecl &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Decl *rhs) const override {
    return typeid(*rhs) == typeid(ObjectDecl) &&
           *this == *dynamic_cast<const ObjectDecl *>(rhs);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(ObjectDecl) &&
           *this == *dynamic_cast<const ObjectDecl *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return nullptr;
  }

 private:
  Object *object_;
  Initializer *init_;
};

// TODO(dxy): link function prototype and function definition
class FuncDecl : public Decl {
 public:
  explicit FuncDecl(Function *func) : func_(func) {}

  [[nodiscard]] Function *GetFunc() const {
    return func_;
  }

  std::string GetIdent() override {
    return "";
  }

  bool operator==(const FuncDecl &rhs) const;

  bool operator!=(const FuncDecl &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Decl *rhs) const override {
    return typeid(*rhs) == typeid(FuncDecl) &&
           *this == *dynamic_cast<const FuncDecl *>(rhs);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(FuncDecl) &&
           *this == *dynamic_cast<const FuncDecl *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override;

 private:
  Function *func_;
};

/**
 * struct, union, enum and typedef
 */
class TypeDecl : public Decl {
 public:
  explicit TypeDecl(TypeDeclType *type) : type_(type) {}

  [[nodiscard]] TypeDeclType *GetType() const {
    return type_;
  }

  std::string GetIdent() override;

  bool operator==(const TypeDecl &rhs) const;

  bool operator!=(const TypeDecl &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Decl *rhs) const override {
    return typeid(*rhs) == typeid(TypeDecl) &&
           *this == *dynamic_cast<const TypeDecl *>(rhs);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(TypeDecl) &&
           *this == *dynamic_cast<const TypeDecl *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return nullptr;
  }

 private:
  TypeDeclType *type_;
};
}

#endif // CCOMPILER_DECLARATION_H