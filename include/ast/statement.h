//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_Stmt_H
#define CCOMPILER_Stmt_H

#include <list>
#include <memory>

namespace CCompiler {
class Expr;

class Identifier;

class Stmt {
};

using StmtPtr = std::unique_ptr<Stmt>;
using StmtList = std::list<StmtPtr>;

class CompoundStmt : public Stmt {
 private:
  StmtList stmts_;  // include declarations and statements
};

class IfStmt : public CompoundStmt {
 private:
  std::unique_ptr<Expr> condition_;
  StmtPtr else_;
};

class LabelStmt : public CompoundStmt {
 private:
  union Label {
    std::unique_ptr<Identifier> identifier_;  // common label
    int value_;  // case label
    bool default_;  // default label
  };
  Label label_;
};

class SwitchStmt : public CompoundStmt {
 private:
  std::unique_ptr<Expr> condition_;
  std::unique_ptr<LabelStmt> labels_;
};

class WhileStmt : public CompoundStmt {
 private:
  std::unique_ptr<Expr> condition_;
};

class DoWhileStmt : public CompoundStmt {
 private:
  std::unique_ptr<Expr> condition_;
};

class ForStmt : public CompoundStmt {
 private:
  StmtPtr initializer_;
  std::unique_ptr<Expr> condition_;
  StmtPtr after_loop_;
};

class JumpStmt : public Stmt {
 private:
  enum class JumpKeyWord {
    kGoto,
    kContinue,
    kBreak,
    kReturn
  };
  JumpKeyWord jump_;
};
}

#endif // CCOMPILER_Stmt_H