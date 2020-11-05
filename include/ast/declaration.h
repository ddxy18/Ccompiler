//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_DECLARATION_H
#define CCOMPILER_DECLARATION_H

#include "ast/identifier.h"
#include "ast/statement.h"

namespace CCompiler {
class DerivedType;

class Expr;

class Function;

class Initializer;

class Object;

class Parser;

class Type;

class Decl : public Stmt {
 public:
  virtual ~Decl() = default;
};

class Initializer {
 public:
  /**
   * Indicate relative location of an element in an array or a member in a
   * struct or an union.
   */
  union Element {
    int offset_;
    std::string ident_;

    explicit Element(int offset) : offset_(offset) {}

    explicit Element(std::string ident) : ident_(std::move(ident)) {}
  };

  explicit Initializer(Element *offset) : offset_(offset) {}

 private:
  Element *offset_;
};

class BaseInitializer : public Initializer {
 public:
  BaseInitializer(Element *offset, Expr *init_value)
          : Initializer(offset),
            init_value_(init_value) {}

 private:
  Expr *init_value_;
};

class InitializerList : public Initializer {
 public:
  explicit InitializerList(Element *offset,
                           std::list<Initializer *> init_list = std::list<Initializer *>())
          : Initializer(offset),
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
  ObjectDecl(Object *object, Initializer *initializer)
          : object_(object),
            init_(initializer) {
    object_->decl_ = this;
  }

 private:
  Object *object_;
  Initializer *init_;
};

class FuncDecl : public Decl {
 public:
  explicit FuncDecl(Function *func) : func_(func) {}

  [[nodiscard]] Function *GetFunc() const {
    return func_;
  }

 private:
  Function *func_;
};

/**
 * All types can be declared. But for any type other than struct, union, enum
 * and typedef, this type declaration has no actual effect and will be
 * discarded soon.
 */
class TypeDecl : public Decl {
 public:
  explicit TypeDecl(Type *type) : type_(type) {}

 private:
  Type *type_;
};
}

#endif // CCOMPILER_DECLARATION_H