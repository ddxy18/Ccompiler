//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_EXPRESSION_H
#define CCOMPILER_EXPRESSION_H

#include <list>
#include <memory>
#include <string>

namespace CCompiler {
class Function;

class Object;

class Expr {
 private:
  enum Op {
  };
  Op op_;
};

class UnaryExpr : public Expr {
 private:
  std::unique_ptr<Expr> op_;
};

class BinaryExpr : public Expr {
 private:
  std::unique_ptr<Expr> left_op_;
  std::unique_ptr<Expr> right_op_;
};

class FuncCall : public Expr {
 private:
  using ParamList = std::list<std::unique_ptr<Expr>>;
  ParamList params_;
  std::unique_ptr<Function> func_;
};

class Constant : public Expr {
 private:
  union Const {
    int integer_;
    double float_;
    std::string literal_;
  };
  Const const_;
};
}

#endif // CCOMPILER_EXPRESSION_H