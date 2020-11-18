//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_EXPRESSION_H
#define CCOMPILER_EXPRESSION_H

#include <list>
#include <string>
#include <utility>
#include <variant>

#include "lex/token.h"

namespace CCompiler {
class FuncDecl;

class Function;

class Object;

/**
 * All identifiers exclude function identifiers in the expressions are wrapped
 * in Object class. So we can use Expr to represent the operands consistently.
 */
class Expr {
 public:
  explicit Expr(TokenType op) : op_(op) {}

  virtual ~Expr() = default;

  virtual bool IsIntConstant() {
    return false;
  }

  /**
   * User must use IsIntConstant() to check whether it is an integer constant
   * firstly.
   * @return
   */
  virtual int ToInt() {
    exit(-1);
  }

  bool operator==(const Expr &rhs) const {
    return op_ == rhs.op_;
  }

  bool operator!=(const Expr &rhs) const {
    return !(*this == rhs);
  }

  virtual bool Equal(const Expr *rhs) const {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(Expr) && *this == *rhs;
  }

 protected:
  TokenType op_;
};

class UnaryExpr : public Expr {
 public:
  UnaryExpr(TokenType op, Expr *operand, bool is_back = false)
          : Expr(op),
            operand_(operand),
            is_back_(is_back) {}

  bool IsIntConstant() override {
    if (operand_->IsIntConstant() &&
        (op_ == TokenType::kSizeof || op_ == TokenType::k_Alignof)) {
      return true;
    }
    return false;
  }

  int ToInt() override {
    // TODO(dxy):
    return 0;
  }

  bool operator==(const UnaryExpr &rhs) const {
    if (operand_ == nullptr) {
      return Expr::operator==(rhs) &&
             rhs.operand_ == nullptr &&
             is_back_ == rhs.is_back_;
    }
    return Expr::operator==(rhs) &&
           operand_->Equal(rhs.operand_) &&
           is_back_ == rhs.is_back_;
  }

  bool operator!=(const UnaryExpr &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Expr *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(UnaryExpr) &&
           *this == *dynamic_cast<const UnaryExpr *>(rhs);
  }

 private:
  Expr *operand_;
  bool is_back_;  // whether put behind the operand
};

class BinaryExpr : public Expr {
 public:
  BinaryExpr(TokenType op, Expr *left_operand, Expr *right_operand)
          : Expr(op),
            l_operand_(left_operand),
            r_operand_(right_operand) {}

  bool IsIntConstant() override {
    if ((l_operand_->IsIntConstant() && r_operand_->IsIntConstant()) &&
        (op_ != TokenType::kComma &&
         op_ != TokenType::kAssign &&
         op_ != TokenType::kMultiAssign &&
         op_ != TokenType::kDivideAssign &&
         op_ != TokenType::kMinusAssign &&
         op_ != TokenType::kPlusAssign &&
         op_ != TokenType::kModuloAssign &&
         op_ != TokenType::kOrAssign &&
         op_ != TokenType::kXorAssign &&
         op_ != TokenType::kAndAssign &&
         op_ != TokenType::kLeftShiftAssign &&
         op_ != TokenType::kRightShiftAssign)) {
      return true;
    }
    return false;
  }

  int ToInt() override {
    if (op_ == TokenType::kLogicalOr) {
      return l_operand_->ToInt() || r_operand_->ToInt();
    } else if (op_ == TokenType::kLogicalAnd) {
      return l_operand_->ToInt() && r_operand_->ToInt();
    } else if (op_ == TokenType::kBitOr) {
      return l_operand_->ToInt() | r_operand_->ToInt();
    } else if (op_ == TokenType::kBitXor) {
      return l_operand_->ToInt() ^ r_operand_->ToInt();
    } else if (op_ == TokenType::kBitAnd) {
      return l_operand_->ToInt() & r_operand_->ToInt();
    } else if (op_ == TokenType::kEqual) {
      return l_operand_->ToInt() == r_operand_->ToInt();
    } else if (op_ == TokenType::kNotEqual) {
      return l_operand_->ToInt() != r_operand_->ToInt();
    } else if (op_ == TokenType::kLess) {
      return l_operand_->ToInt() < r_operand_->ToInt();
    } else if (op_ == TokenType::kMore) {
      return l_operand_->ToInt() > r_operand_->ToInt();
    } else if (op_ == TokenType::kLessEqual) {
      return l_operand_->ToInt() <= r_operand_->ToInt();
    } else if (op_ == TokenType::kMoreEqual) {
      return l_operand_->ToInt() >= r_operand_->ToInt();
    } else if (op_ == TokenType::kLeftShift) {
      return l_operand_->ToInt() << r_operand_->ToInt();
    } else if (op_ == TokenType::kRightShift) {
      return l_operand_->ToInt() >> r_operand_->ToInt();
    } else if (op_ == TokenType::kPlus) {
      return l_operand_->ToInt() + r_operand_->ToInt();
    } else if (op_ == TokenType::kMinus) {
      return l_operand_->ToInt() - r_operand_->ToInt();
    } else if (op_ == TokenType::kAsterisk) {
      return l_operand_->ToInt() * r_operand_->ToInt();
    } else if (op_ == TokenType::kDivide) {
      return l_operand_->ToInt() / r_operand_->ToInt();
    } else if (op_ == TokenType::kModulo) {
      return l_operand_->ToInt() % r_operand_->ToInt();
    }
  }

  bool operator==(const BinaryExpr &rhs) const {
    bool l_operand_equal, r_operand_equal;

    if (l_operand_ == nullptr) {
      l_operand_equal = rhs.l_operand_ == nullptr;
    } else {
      l_operand_equal = l_operand_->Equal(rhs.l_operand_);
    }
    if (r_operand_ == nullptr) {
      r_operand_equal = rhs.r_operand_ == nullptr;
    } else {
      r_operand_equal = r_operand_->Equal(rhs.r_operand_);
    }

    return Expr::operator==(rhs) && l_operand_equal && r_operand_equal;
  }

  bool operator!=(const BinaryExpr &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Expr *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(BinaryExpr) &&
           *this == *dynamic_cast<const BinaryExpr *>(rhs);
  }

 private:
  Expr *l_operand_;
  Expr *r_operand_;
};

class ConditionalExpr : public Expr {
 public:
  ConditionalExpr(TokenType op, Expr *operand1, Expr *operand2, Expr *operand3)
          : Expr(op),
            operand1_(operand1),
            operand2_(operand2),
            operand3_(operand3) {}

  bool IsIntConstant() override {
    return false;
  }

  int ToInt() override {
    exit(-1);
  }

  bool operator==(const ConditionalExpr &rhs) const {
    bool operand1_equal, operand2_equal, operand3_equal;

    if (operand1_ == nullptr) {
      operand1_equal = rhs.operand1_ == nullptr;
    } else {
      operand1_equal = operand1_->Equal(rhs.operand1_);
    }
    if (operand2_ == nullptr) {
      operand2_equal = rhs.operand2_ == nullptr;
    } else {
      operand2_equal = operand2_->Equal(rhs.operand2_);
    }
    if (operand3_ == nullptr) {
      operand3_equal = rhs.operand3_ == nullptr;
    } else {
      operand3_equal = operand3_->Equal(rhs.operand3_);
    }

    return Expr::operator==(rhs) &&
           operand1_equal &&
           operand2_equal &&
           operand3_equal;
  }

  bool operator!=(const ConditionalExpr &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Expr *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(ConditionalExpr) &&
           *this == *dynamic_cast<const ConditionalExpr *>(rhs);
  }

 private:
  Expr *operand1_;
  Expr *operand2_;
  Expr *operand3_;
};

class ArrayExpr : public Expr {
 public:
  ArrayExpr(Expr *base, Expr *index)
          : Expr(TokenType::kEmpty),
            base_(base),
            index_(index) {}

  bool IsIntConstant() override {
    return false;
  }

  int ToInt() override {
    exit(-1);
  }

  bool operator==(const ArrayExpr &rhs) const {
    bool base_equal, index_equal;

    if (base_ == nullptr) {
      base_equal = base_ == nullptr;
    } else {
      base_equal = base_->Equal(rhs.base_);
    }
    if (index_ == nullptr) {
      index_equal = index_ == nullptr;
    } else {
      index_equal = index_->Equal(rhs.index_);
    }

    return Expr::operator==(rhs) && base_equal && index_equal;
  }

  bool operator!=(const ArrayExpr &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Expr *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(ArrayExpr) &&
           *this == *dynamic_cast<const ArrayExpr *>(rhs);
  }

 private:
  Expr *base_;
  Expr *index_;
};

class FuncCall : public Expr {
 public:
  using ParamList = std::list<Expr *>;

  FuncCall(Function *func, ParamList params)
          : Expr(TokenType::kEmpty),
            func_(func),
            params_(std::move(params)) {}

  bool IsIntConstant() override {
    return false;
  }

  int ToInt() override {
    exit(-1);
  }

  bool operator==(const FuncCall &rhs) const;

  bool operator!=(const FuncCall &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Expr *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(FuncCall) &&
           *this == *dynamic_cast<const FuncCall *>(rhs);
  }

 private:
  Function *func_;
  ParamList params_;
};

class Constant : public Expr {
 public:
  using Const = std::variant<int, float, char, std::string>;

  explicit Constant(int constant)
          : Expr(TokenType::kEmpty),
            const_(constant) {}

  explicit Constant(float constant)
          : Expr(TokenType::kEmpty),
            const_(constant) {}

  explicit Constant(char constant)
          : Expr(TokenType::kEmpty),
            const_(constant) {}

  explicit Constant(std::string literal)
          : Expr(TokenType::kEmpty),
            const_(literal) {}

  bool IsIntConstant() override {
    // TODO(dxy): floating constants can be cast to integer constants
    if (const_.index() == 0) {
      return true;
    }
    return false;
  }

  int ToInt() override {
    return std::get<int>(const_);
  }

  [[nodiscard]] const Const &GetConst() const {
    return const_;
  }

  bool operator==(const Constant &rhs) const {
    return Expr::operator==(rhs) && const_ == rhs.const_;
  }

  bool operator!=(const Constant &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Expr *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(Constant) &&
           *this == *dynamic_cast<const Constant *>(rhs);
  }

 private:
  Const const_;
};
}

#endif // CCOMPILER_EXPRESSION_H