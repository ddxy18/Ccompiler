//
// Created by dxy on 2020/11/9.
//

#ifndef CCOMPILER_SCOPE_H
#define CCOMPILER_SCOPE_H

#include <list>
#include <string>

#include "ast/list_util.h"

namespace CCompiler {
class Decl;

class ObjectDecl;

class FuncDecl;

class TypeDecl;

class Expr;

class Function;

class Object;

class TypeDeclType;

/**
 * It stores object, function and type declarations. We combine some
 * identifiers to a whole since they are always dealt with together. Moreover
 * label identifiers are not included here and they will be dealt with
 * separately.
 */
class Scope {
 public:
  enum class ScopeType {
    kFile,
    kBlock,
    kFunc,
    kFuncPrototype
  };

  Scope(ScopeType type, Scope *parent)
          : type_(type),
            parent_(parent) {}

  void AddIdent(ObjectDecl *obj_decl);

  /**
   * Function definition and prototype only exists in the file scope, so we
   * just push the func_decl without checking and leave the work of checking
   * its validity to the only user to complete.
   * @param func_decl
   */
  void AddIdent(FuncDecl *func_decl);

  void AddIdent(TypeDecl *type_decl);

  /**
   * Find an object that has the name ident from the current scope to the
   * outer scope.
   * @param ident cannot be ""
   * @return The object that is declared nearest the current scope. If no
   * valid object exists across reachable scopes, return nullptr.
   */
  Object *GetObject(const std::string &ident);

  /**
   * It is similar to the GetObject(const std::string &) by checking the tag.
   * @param ident
   * @return
   */
  TypeDeclType *GetType(const std::string &ident);

  /**
   * @param ident cannot be ""
   * @return A function that have the name ident. Otherwise return nullptr.
   */
  Function *GetFunc(const std::string &ident);

  [[nodiscard]] Scope *GetParent() const {
    return parent_;
  }

  bool operator==(const Scope &rhs) const {
    if (parent_ == nullptr) {
      return type_ == rhs.type_ &&
             rhs.parent_ == nullptr &&
             CCompiler::Equal(decl_list_, rhs.decl_list_);
    }
    return type_ == rhs.type_ &&
           parent_->Equal(rhs.parent_) &&
           CCompiler::Equal(decl_list_, rhs.decl_list_);
  }

  bool operator!=(const Scope &rhs) const {
    return !(*this == rhs);
  }

  virtual bool Equal(const Scope *rhs) const {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(Scope) && *this == *rhs;
  }

 private:
  ScopeType type_;
  Scope *parent_;
  std::list<Decl *> decl_list_;
};
}

#endif // CCOMPILER_SCOPE_H