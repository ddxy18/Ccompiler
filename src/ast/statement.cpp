//
// Created by dxy on 2020/11/13.
//

#include "ast/statement.h"

#include "ast/declaration.h"
#include "ast/list_util.h"
#include "ast/scope.h"
#include "ast/expression.h"

using namespace CCompiler;
using namespace std;

bool ExprStmt::operator==(const ExprStmt &rhs) const {
  if (expr_ == nullptr) {
    return rhs.expr_ == nullptr;
  }
  return expr_->Equal(rhs.expr_);
}

bool IfStmt::operator==(const IfStmt &rhs) const {
  bool condition_equal, else_stmt_equal;
  if (condition_ == nullptr) {
    condition_equal = rhs.condition_ == nullptr;
  } else {
    condition_equal = condition_->Equal(rhs.condition_);
  }
  if (else_stmt_ == nullptr) {
    else_stmt_equal = rhs.else_stmt_ == nullptr;
  } else {
    else_stmt_equal = else_stmt_->Equal(rhs.else_stmt_);
  }

  return CompoundStmt::operator==(rhs) && condition_equal && else_stmt_equal;
}

bool SwitchStmt::operator==(const SwitchStmt &rhs) const {
  if (condition_ == nullptr) {
    return CompoundStmt::operator==(rhs) && rhs.condition_ == nullptr;
  }
  return CompoundStmt::operator==(rhs) && condition_->Equal(rhs.condition_);
}

bool WhileStmt::operator==(const WhileStmt &rhs) const {
  if (condition_ == nullptr) {
    return CompoundStmt::operator==(rhs) &&
           rhs.condition_ == nullptr &&
           is_condition_first_ == rhs.is_condition_first_;
  }
  return CompoundStmt::operator==(rhs) &&
         condition_->Equal(rhs.condition_) &&
         is_condition_first_ == rhs.is_condition_first_;
}

bool ForStmt::operator==(const ForStmt &rhs) const {
  bool condition_equal, after_loop_equal;

  if (condition_ == nullptr) {
    condition_equal = rhs.condition_ == nullptr;
  } else {
    condition_equal = condition_->Equal(rhs.condition_);
  }
  if (after_loop_ == nullptr) {
    after_loop_equal = rhs.after_loop_ == nullptr;
  } else {
    after_loop_equal = after_loop_->Equal(rhs.after_loop_);
  }
  return CompoundStmt::operator==(rhs) &&
         CCompiler::Equal(initializer_, rhs.initializer_) &&
         condition_equal &&
         after_loop_equal;
}

bool LabelStmt::operator==(const LabelStmt &rhs) const {
  return label_ == rhs.label_ && CCompiler::Equal(stmt_list_, rhs.stmt_list_);
}

bool CompoundStmt::operator==(const CompoundStmt &rhs) const {
  if (scope_ == nullptr) {
    return CCompiler::Equal(stmts_, rhs.stmts_) && rhs.scope_ == nullptr;
  }
  return CCompiler::Equal(stmts_, rhs.stmts_) && scope_->Equal(rhs.scope_);
}

bool CCompiler::operator==(const JumpStmt &lhs, const JumpStmt &rhs) {
  return lhs.jump_ == rhs.jump_ && lhs.ident_ == rhs.ident_;
}

bool ReturnStmt::operator==(const ReturnStmt &rhs) const {
  if (return_ == nullptr) {
    return CCompiler::operator==(*dynamic_cast<const JumpStmt *>(this),
                                 dynamic_cast<const JumpStmt &>(rhs)) &&
           rhs.return_ == nullptr;
  }
  return CCompiler::operator==(*dynamic_cast<const JumpStmt *>(this),
                               dynamic_cast<const JumpStmt &>(rhs)) &&
         return_->Equal(rhs.return_);
}