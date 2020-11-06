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
 public:
  // TODO(dxy):
  IfStmt(Expr *condition, Stmt *if_stmt, Stmt *else_stmt)
          : condition_(condition),
            else_(else_stmt) {}

 private:
  // If statement is stored in base class.
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
 public:
  // TODO(dxy): initialize base class
  SwitchStmt(Expr *condition, Stmt *stmt) : condition_(condition) {}

 private:
  Expr *condition_;
};

class WhileStmt : public CompoundStmt {
 public:
  // TODO(dxy): initialize base class
  WhileStmt(Expr *condition, Stmt *stmt) : condition_(condition) {}

 private:
  Expr *condition_;
};

class DoWhileStmt : public CompoundStmt {
 public:
  // TODO(dxy): initialize base class
  DoWhileStmt(Expr *condition, Stmt *stmt) : condition_(condition) {}

 private:
  Expr *condition_;
};

class ForStmt : public CompoundStmt {
 public:
  ForStmt(std::list<Stmt *> init, Expr *condition, Stmt *after_loop, Stmt *body)
          : initializer_(std::move(init)),
            condition_(condition),
            after_loop_(after_loop) {}

 private:
  std::list<Stmt *> initializer_;
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

class ReturnStmt : public JumpStmt {
 public:
  explicit ReturnStmt(Expr *return_value) :
          JumpStmt(JumpType::kReturn),
          return_(return_value) {}

 private:
  Expr *return_;
};
}

#endif // CCOMPILER_Stmt_H