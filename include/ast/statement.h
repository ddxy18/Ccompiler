//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_Stmt_H
#define CCOMPILER_Stmt_H

#include <list>
#include <string>
#include <utility>
#include <variant>

namespace CCompiler {
class Expr;

class Identifier;

class Scope;

/**
 * This is a base class for most of the nodes in AST,
 */
class Stmt {
 public:
  virtual ~Stmt() = default;

  bool operator==(const Stmt &rhs) const {
    return true;
  }

  bool operator!=(const Stmt &rhs) const {
    return !(rhs == *this);
  }

  virtual bool Equal(const Stmt *rhs) const = 0;

  [[nodiscard]] virtual Scope *GetOwnedScope() const = 0;
};

using StmtList = std::list<Stmt *>;

class ExprStmt : public Stmt {
 public:
  explicit ExprStmt(Expr *expr) : expr_(expr) {}

  bool operator==(const ExprStmt &rhs) const;

  bool operator!=(const ExprStmt &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(ExprStmt) &&
           *this == *dynamic_cast<const ExprStmt *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return nullptr;
  }

 private:
  Expr *expr_;
};

/**
 * Only the body for a function will declare a CompoundStmt object. {xxx} for
 * while, for, if and switch statement are split to a list of statements.
 */
class CompoundStmt : public Stmt {
  friend class ForStmt;

 public:
  explicit CompoundStmt(Scope *scope) : scope_(scope) {}

  CompoundStmt(Scope *scope, StmtList stmts)
          : scope_(scope),
          stmts_(std::move(stmts)) {}

  void AddStmt(Stmt *stmt) {
    stmts_.push_back(stmt);
  }

  bool operator==(const CompoundStmt &rhs) const;

  bool operator!=(const CompoundStmt &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(CompoundStmt) &&
           *this == *dynamic_cast<const CompoundStmt *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return scope_;
  }

 private:
  StmtList stmts_;  //!< include declarations and statements
  Scope *scope_;
};

class IfStmt : public CompoundStmt {
 public:
  /**
   * if(condition) if_stmt
   * if(condition) if_stmt else else_stmt
   *
   * @param if_scope
   * @param if_stmt
   * @param condition
   * @param else_stmt
   */
  IfStmt(Scope *if_scope, const StmtList &if_stmt,
         Expr *condition,
         CompoundStmt *else_stmt = nullptr)
          : CompoundStmt(if_scope, if_stmt),
            condition_(condition),
            else_stmt_(else_stmt) {}

  bool operator==(const IfStmt &rhs) const;

  bool operator!=(const IfStmt &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(IfStmt) &&
           *this == *dynamic_cast<const IfStmt *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return CompoundStmt::GetOwnedScope();
  }

 private:
  // If statement is stored in base class.

  Expr *condition_;
  /**
   * Since else statement has its own scope, so we require a CompoundStmt *
   * object to represent it.
   */
  CompoundStmt *else_stmt_;
};

class LabelStmt : public Stmt {
 public:
  /**
   * Identifier *--common label
   * int--case label in switch
   * bool--default label in switch
   */
  using Label = std::variant<Identifier *, int, bool>;

  LabelStmt(Label label, StmtList stmt_list)
          : label_(label),
            stmt_list_(std::move(stmt_list)) {}

  bool operator==(const LabelStmt &rhs) const;

  bool operator!=(const LabelStmt &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(LabelStmt) &&
           *this == *dynamic_cast<const LabelStmt *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return nullptr;
  }

 private:
  Label label_;
  StmtList stmt_list_;
};

class SwitchStmt : public CompoundStmt {
 public:
  SwitchStmt(Scope *scope, Expr *condition, const StmtList &stmt_list)
          : CompoundStmt(scope, stmt_list),
            condition_(condition) {}

  bool operator==(const SwitchStmt &rhs) const;

  bool operator!=(const SwitchStmt &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(SwitchStmt) &&
           *this == *dynamic_cast<const SwitchStmt *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return CompoundStmt::GetOwnedScope();
  }

 private:
  Expr *condition_;
};

class WhileStmt : public CompoundStmt {
 public:
  /**
   * while (condition)
   *   stmt_list
   *
   * @param scope
   * @param condition
   * @param stmt_list never contain a CompoundStmt * object
   * @param is_condition_first true--while, false--do while
   */
  WhileStmt(Scope *scope, const StmtList &stmt_list,
            Expr *condition,
            bool is_condition_first)
          : CompoundStmt(scope, stmt_list),
            condition_(condition),
            is_condition_first_(is_condition_first) {}

  bool operator==(const WhileStmt &rhs) const;

  bool operator!=(const WhileStmt &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(WhileStmt) &&
           *this == *dynamic_cast<const WhileStmt *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return CompoundStmt::GetOwnedScope();
  }

 private:
  bool is_condition_first_;  // true--while, false--do while
  Expr *condition_;
};

class ForStmt : public CompoundStmt {
 public:
  ForStmt(Scope *scope, const StmtList &body,
          StmtList init,
          Expr *condition,
          Expr *after_loop)
          : CompoundStmt(scope, body),
            initializer_(std::move(init)),
            condition_(condition),
            after_loop_(after_loop) {}

  bool operator==(const ForStmt &rhs) const;

  bool operator!=(const ForStmt &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(ForStmt) &&
           *this == *dynamic_cast<const ForStmt *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return CompoundStmt::GetOwnedScope();
  }

 private:
  //!< before first ';'. We may have several declarations here so we use a
  //!< list to contain them.
  StmtList initializer_;
  Expr *condition_;  //!< between first ';' and second ';'
  Expr *after_loop_;  //!< after second ';'
};

class JumpStmt : public Stmt {
  friend bool operator==(const JumpStmt &lhs, const JumpStmt &rhs);

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

  virtual ~JumpStmt() = default;

  bool operator!=(const JumpStmt &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(JumpStmt) &&
           *this == *dynamic_cast<const JumpStmt *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return nullptr;
  }

 private:
  JumpType jump_;
  std::string ident_;  //!< only for goto statement
};

bool operator==(const JumpStmt &lhs, const JumpStmt &rhs);

class ReturnStmt : public JumpStmt {
 public:
  explicit ReturnStmt(Expr *return_value)
          : JumpStmt(JumpType::kReturn),
            return_(return_value) {}

  bool operator==(const ReturnStmt &rhs) const;

  bool operator!=(const ReturnStmt &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Stmt *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(ReturnStmt) &&
           *this == *dynamic_cast<const ReturnStmt *>(rhs);
  }

  [[nodiscard]] Scope *GetOwnedScope() const override {
    return nullptr;
  }

 private:
  Expr *return_;
};
}

#endif // CCOMPILER_Stmt_H