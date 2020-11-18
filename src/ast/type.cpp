//
// Created by dxy on 2020/11/13.
//

#include "ast/type.h"

#include "ast/identifier.h"
#include "ast/list_util.h"

using namespace CCompiler;
using namespace std;

bool CCompiler::operator==(const TypeDeclType &lhs, const TypeDeclType &rhs) {
  return lhs.ident_ == rhs.ident_;
}

bool StructUnionType::operator==(const StructUnionType &rhs) const {
  return CCompiler::operator==(*dynamic_cast<const TypeDeclType *>(this),
                               dynamic_cast<const TypeDeclType &>(rhs)) &&
         flag_ == rhs.flag_ &&
         CCompiler::Equal(members_, rhs.members_);
}

bool FuncType::operator==(const FuncType &rhs) const {
  return DerivedType::operator==(rhs) &&
         CCompiler::Equal(params_, rhs.params_);
}
