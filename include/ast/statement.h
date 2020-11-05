//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_Stmt_H
#define CCOMPILER_Stmt_H

#include <list>
#include <utility>

namespace CCompiler {
class Expr;

class Identifier;

class Stmt {
};

using StmtList = std::list<Stmt *>;

class CompoundStmt : public Stmt {
 public:
  CompoundStmt() = default;

  void AddStmt(Stmt *stmt) {
    stmts_.push_back(stmt);
  }

 private:
  StmtList stmts_;  // include declarations and statements
};

class IfStmt : public CompoundStmt {
 private:
  Expr *condition_;
  Stmt *else_;
};

class LabelStmt : public CompoundStmt {
 public:
  union Label {
    Identifier *identifier_;  // common label
    int value_;  // case label in switch
    bool default_{false};  // default label in switch

    explicit Label(Identifier *identifier) : identifier_(identifier) {}

    explicit Label(int value) : value_(value) {}

    /**
     * @brief Initialize default member
     */
    explicit Label() : default_(true) {}
  };

  // TODO(dxy): initialize base class
  explicit LabelStmt(Label *label, Stmt *stmt) : label_(label) {}

 private:
  Label *label_;
};

class SwitchStmt : public CompoundStmt {
 private:
  Expr *condition_;
  LabelStmt *labels_;
};

class WhileStmt : public CompoundStmt {
 private:
  Expr *condition_;
};

class DoWhileStmt : public CompoundStmt {
 private:
  Expr *condition_;
};

class ForStmt : public CompoundStmt {
 private:
  Stmt *initializer_;
  Expr *condition_;
  Stmt *after_loop_;
};

class JumpStmt : public Stmt {
 public:
  enum class JumpType {
    kGoto,
    kContinue,
    kBreak,
    kReturn
  };

  explicit JumpStmt(JumpType jump, std::string ident = "")
          : jump_(jump),
            ident_(std::move(ident)) {}

 private:
  JumpType jump_;
  std::string ident_;  // only for goto statement
};
}

#endif // CCOMPILER_Stmt_H