//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_Stmt_H
#define CCOMPILER_Stmt_H

#include <list>
#include <memory>

namespace CCompiler {
class Stmt {
};

using StmtPtr = std::shared_ptr<Stmt>;
using StmtList = std::list<StmtPtr>;

class CompoundStmt : public Stmt {
 private:
  StmtList stmt_list_;  // include declarations and statements
};

class IfStmt : public CompoundStmt {
};

class SwitchStmt : public CompoundStmt {
};

class WhileStmt : public CompoundStmt {
};

class DoWhileStmt : public CompoundStmt {
};

class ForStmt : public CompoundStmt {
};

class LabelStmt : public CompoundStmt {
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