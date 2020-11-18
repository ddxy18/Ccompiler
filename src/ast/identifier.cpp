//
// Created by dxy on 2020/11/14.
//

#include "ast/identifier.h"

#include "ast/declaration.h"
#include "ast/list_util.h"
#include "ast/type.h"

using namespace CCompiler;
using namespace ::std;


bool Identifier::operator==(const Identifier &rhs) const {
  if (type_ == nullptr) {
    return rhs.type_ == nullptr &&
           ident_ == rhs.ident_ &&
           linkage_ == rhs.linkage_;
  }
  return type_->Equal(rhs.type_) &&
         ident_ == rhs.ident_ &&
         linkage_ == rhs.linkage_;
}

bool Object::operator==(const Object &rhs) const {
  if (decl_ == nullptr) {
    return Identifier::operator==(rhs) &&
           Expr::operator==(rhs) &&
           storage_ == rhs.storage_ &&
           rhs.decl_ == nullptr;
  }
  return Identifier::operator==(rhs) &&
         Expr::operator==(rhs) &&
         storage_ == rhs.storage_ &&
         decl_->Equal(rhs.decl_);
}

bool Function::operator==(const Function &rhs) const {
  if (body_ == nullptr) {
    return Identifier::operator==(rhs) &&
           Expr::operator==(rhs) &&
           CCompiler::Equal(params_, rhs.params_) &&
           rhs.body_ == nullptr;
  }
  return Identifier::operator==(rhs) &&
         Expr::operator==(rhs) &&
         CCompiler::Equal(params_, rhs.params_) &&
         body_->Equal(rhs.body_);
}

[[nodiscard]] Scope *Function::GetOwnedScope() const {
  return body_->GetOwnedScope();
}