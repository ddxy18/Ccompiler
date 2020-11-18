//
// Created by dxy on 2020/10/24.
//

#ifndef CCOMPILER_TRANSLATION_UNIT_H
#define CCOMPILER_TRANSLATION_UNIT_H

#include <list>
#include <string>

#include "ast/scope.h"
#include "ast/identifier.h"

namespace CCompiler {
class Decl;

/**
 * A translation unit contains either declarations or function definitions.
 * It usually refers to a source file in C.
 */
class TranslationUnit {
 public:
  TranslationUnit()
          : file_scope_(new Scope(Scope::ScopeType::kFile, nullptr)) {}

  void AddExternalDef(ObjectDecl *obj_decl) {
    if (file_scope_->GetObject(obj_decl->GetIdent()) == nullptr) {
      file_scope_->AddIdent(obj_decl);
    } else {
      exit(-1);
    }
  }

  void AddExternalDef(FuncDecl *func_decl) {
    auto func = dynamic_cast<FuncDecl *>(func_decl)->GetFunc();
    auto func_in_scope = file_scope_->GetFunc(func_decl->GetIdent());
    if (func_in_scope == nullptr) {
      file_scope_->AddIdent(func_decl);
    } else if (func_in_scope->Equal(dynamic_cast<const Identifier *>(func))) {
      if (func_in_scope->IsDefined()) {  // decl must be a function prototype
        if (func->IsDefined()) {  // multiple definition
          exit(-1);
        }
      } else {
        if (func->IsDefined()) {
          func_in_scope->BodyInit(func);
        }
      }
      delete func;
    } else {
      exit(-1);
    }
  }

  void AddExternalDef(TypeDecl *type_decl) {
    // TODO(dxy): typedef permits multiple definitions
    // type declared with no tag doesn't have to consider redefinition
    if (!type_decl->GetIdent().empty()) {
      if (file_scope_->GetType(type_decl->GetIdent()) != nullptr) {
        exit(-1);
      }
    }
    file_scope_->AddIdent(type_decl);
  }

  [[nodiscard]] Scope *GetScope() const {
    return file_scope_;
  }

  bool operator==(const TranslationUnit &rhs) const {
    if (file_scope_ == nullptr) {
      return rhs.file_scope_ == nullptr;
    }
    return file_scope_->Equal(rhs.file_scope_);
  }

  virtual bool Equal(const TranslationUnit *rhs) const {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(TranslationUnit) && *this == *rhs;
  }

 private:
  Scope *file_scope_;
};
}

#endif // CCOMPILER_TRANSLATION_UNIT_H