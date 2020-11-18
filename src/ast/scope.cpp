//
// Created by dxy on 2020/11/11.
//

#include "ast/scope.h"

#include "ast/declaration.h"

using namespace CCompiler;
using namespace std;

Object *Scope::GetObject(const string &ident) {
  // find an object in the current scope
  for (auto &decl:decl_list_) {
    if (decl->GetIdent() == ident) {
      if (typeid(*decl) == typeid(ObjectDecl)) {
        return dynamic_cast<ObjectDecl *>(decl)->GetObject();
      }
    }
  }
  // We cannot find an object in the current scope, so we try to find it in
  // the outer scope.
  if (parent_ != nullptr) {
    return parent_->GetObject(ident);
  }
  // no valid object that has the name ident exists
  return nullptr;
}

TypeDeclType *Scope::GetType(const string &ident) {
  // find an type in the current scope
  for (auto &decl:decl_list_) {
    if (decl->GetIdent() == ident) {
      if (typeid(*decl) == typeid(ObjectDecl)) {
        return dynamic_cast<TypeDecl *>(decl)->GetType();
      }
    }
  }
  // We cannot find an type in the current scope, so we try to find it in
  // the outer scope.
  if (parent_ != nullptr) {
    return parent_->GetType(ident);
  }
  // no valid object that has the name ident exists
  return nullptr;
}

Function *Scope::GetFunc(const string &ident) {
  for (auto &decl:decl_list_) {
    if (decl->GetIdent() == ident && typeid(*decl) == typeid(FuncDecl)) {
      return dynamic_cast<FuncDecl *>(decl)->GetFunc();
    }
  }

  if (parent_ != nullptr) {
    return parent_->GetFunc(ident);
  }

  return nullptr;
}

void Scope::AddIdent(ObjectDecl *obj_decl) {
  for (auto &decl:decl_list_) {
    if (decl->GetIdent() == obj_decl->GetIdent() &&
        typeid(*decl) == typeid(ObjectDecl)) {
      // function is always declared in the file scope, so decl cannot be
      // FuncDecl* type.
      exit(-1);
    }
  }
  decl_list_.push_back(obj_decl);
}

void Scope::AddIdent(FuncDecl *func_decl) {
  decl_list_.push_back(func_decl);
}

void Scope::AddIdent(TypeDecl *type_decl) {
  // TODO(dxy): typedef permits multiple definitions
  // type declared with no tag doesn't have to consider redefinition
  if (!type_decl->GetIdent().empty()) {
    for (auto &decl:decl_list_) {
      if (typeid(*decl) == typeid(TypeDecl) &&
          decl->GetIdent() == type_decl->GetIdent()) {
        exit(-1);
      }
    }
  }
  decl_list_.push_back(type_decl);
}