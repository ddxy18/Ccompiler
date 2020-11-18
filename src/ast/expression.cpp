//
// Created by dxy on 2020/11/13.
//

#include "ast/expression.h"

#include "ast/identifier.h"
#include "ast/list_util.h"

using namespace CCompiler;
using namespace std;

bool FuncCall::operator==(const FuncCall &rhs) const {
  if (func_ == nullptr) {
    return Expr::operator==(rhs) &&
           rhs.func_ == nullptr &&
           CCompiler::Equal(params_, rhs.params_);
  }
  return Expr::operator==(rhs) &&
         func_->Equal(dynamic_cast<const Identifier *>(rhs.func_)) &&
         CCompiler::Equal(params_, rhs.params_);
}