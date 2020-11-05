//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_EXPRESSION_H
#define CCOMPILER_EXPRESSION_H

#include <list>
#include <string>

namespace CCompiler {
class FuncDecl;

class Object;

class Expr {
 private:
  enum Op {
  };
  Op op_;
};

class UnaryExpr : public Expr {
 private:
  Expr* op_;
};

class BinaryExpr : public Expr {
 private:
  Expr* left_op_;
  Expr* right_op_;
};

class FuncCall : public Expr {
 private:
  using ParamList = std::list<Expr*>;
  ParamList params_;
  FuncDecl* func_;
};

class Constant : public Expr {
 public:
  union Const {
    int integer_;
    double float_;
    std::string literal_;
  };
  [[nodiscard]] const Const &GetConst() const {
    return const_;
  }

 private:
  Const const_;
};
}

#endif // CCOMPILER_EXPRESSION_H