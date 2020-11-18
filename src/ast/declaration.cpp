//
// Created by dxy on 2020/11/13.
//

#include "ast/declaration.h"

#include <string>

#include "ast/identifier.h"
#include "ast/list_util.h"
#include "ast/type.h"

using namespace CCompiler;
using namespace std;

ObjectDecl::ObjectDecl(Object *object, Initializer *initializer)
        : object_(object),
          init_(initializer) {
  object_->decl_ = this;
}

string ObjectDecl::GetIdent() {
  return object_->GetIdent();
}

bool ObjectDecl::operator==(const ObjectDecl &rhs) const {
  bool object_equal, init_equal;

  if (object_ == nullptr) {
    object_equal = rhs.object_ == nullptr;
  } else {
    object_equal = object_
            ->Equal(dynamic_cast<const Identifier *>(rhs.object_));
  }
  if (object_ == nullptr) {
    init_equal = rhs.init_ == nullptr;
  } else {
    init_equal = init_->Equal(rhs.init_);
  }

  return object_equal && init_equal;
}

bool FuncDecl::operator==(const FuncDecl &rhs) const {
  if (func_ == nullptr) {
    return rhs.func_ == nullptr;
  }
  return func_->Equal(dynamic_cast<const Identifier *>(rhs.func_));
}

bool InitializerList::operator==(const InitializerList &rhs) const {
  return CCompiler::operator==(*dynamic_cast<const Initializer *>(this),
                               dynamic_cast<const Initializer &>(rhs)) &&
         CCompiler::Equal(init_list_, rhs.init_list_);
}

bool TypeDecl::operator==(const TypeDecl &rhs) const {
  if (type_ == nullptr) {
    return rhs.type_ == nullptr;
  }
  return type_->Equal(rhs.type_);
}

string TypeDecl::GetIdent() {
  return type_->GetIdent();
}

bool BaseInitializer::operator==(const BaseInitializer &rhs) const {
  if (init_value_ == nullptr) {
    return CCompiler::operator==(*dynamic_cast<const Initializer *>(this),
                                 dynamic_cast<const Initializer &>(rhs)) &&
           rhs.init_value_ == nullptr;
  }
  return CCompiler::operator==(*dynamic_cast<const Initializer *>(this),
                               dynamic_cast<const Initializer &>(rhs)) &&
         init_value_->Equal(rhs.init_value_);
}

bool CCompiler::operator==(const Initializer &lhs, const Initializer &rhs) {
  return lhs.offset_ == rhs.offset_;
}

[[nodiscard]] Scope *FuncDecl::GetOwnedScope() const {
  return func_->GetOwnedScope();
}