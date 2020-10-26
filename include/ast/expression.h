//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_EXPRESSION_H
#define CCOMPILER_EXPRESSION_H

namespace CCompiler {
class Expr {
};

class UnaryExpr : public Expr {
};

class BinaryExpr : public Expr {
};

class FuncCall : public Expr {
};

class Constant : public Expr {
};
}

#endif // CCOMPILER_EXPRESSION_H