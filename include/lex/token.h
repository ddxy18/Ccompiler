//
// Created by dxy on 2020/8/13.
//

#ifndef CCOMPILER_TOKEN_H
#define CCOMPILER_TOKEN_H

#include <map>
#include <memory>
#include <string>

namespace CCompiler {
enum class TokenType {
  kAuto,
  kBreak,
  kCase,
  kChar,
  kConst,
  kContinue,
  kDefault,
  kDo,
  kDouble,
  kElse,
  kEnum,
  kExtern,
  kFloat,
  kFor,
  kGoto,
  kIf,
  kInline,
  kInt,
  kLong,
  kRegister,
  kRestrict,
  kReturn,
  kShort,
  kSigned,
  kSizeof,
  kStatic,
  kStruct,
  kSwitch,
  kTypedef,
  kUnion,
  kUnsigned,
  kVoid,
  kVolatile,
  kWhile,
  k_Alignas,
  k_Alignof,
  k_Atomic,
  k_Bool,
  k_Complex,
  k_Generic,
  k_Imaginary,
  k_Noreturn,
  k_Static_assert,
  k_Thread_local,
  kIdentifier,
  kNumber,
  kTilde,  // ~
  kAsterisk,  // *
  kArrow,  // ->
  kDot,  // .
  kQuestion,  // ?
  kMinus,  // -
  kPlus,  // +
  kIncrement,  // ++
  kDecrement,  // --
  kDivide,  // /
  kModulo,  // %
  kAssign,  // =
  kLeftShift,  // <<
  kRightShift,  // >>
  kLess,  // <
  kMore,  // >
  kLessEqual,  // <=
  kMoreEqual,  // >=
  kEqual,  // ==
  kNotEqual,  // !=
  kAnd,  // &
  kOr,  // |
  kXor,  // ^
  kLogicalAnd,  // &&
  kLogicalOr,  // ||
  kLogicalNot,  // !
  kMultiAssign,  // *=
  kDivideAssign,  // /=
  kModuloAssign,  // %=
  kPlusAssign,  // +=
  kMinusAssign,  // -=
  kLeftShiftAssign,  // <<=
  kRightShiftAssign,  // >>=
  kAndAssign,  // &=
  kXorAssign,  // ^=
  kOrAssign,  // |=
  kLeftParenthesis,  // (
  kRightParenthesis,  // )
  kLeftBracket,  // [
  kRightBracket,  // ]
  kLeftCurlyBracket,  // {
  kRightCurlyBracket,  // }
  kComma,  // ,
  kColon,  // :
  kSemicolon,  // ;
  kEllipsis,  // ...
  kNumberSign,  // #
  kCharacter,  // 'x'
  kString,  // "xxx"
  kComment,  // //xxx or /*xxx*/
  kDelim,  // \r \t \n ' '
  kEmpty
};

class Token {
 public:
  explicit Token(std::string token = "", TokenType type = TokenType::kEmpty,
                 int line = 0, int column = 0)
          : token_(std::move(token)),
            type_(type),
            line_(line),
            column_(column) {}

  /**
   * Check token_ to determine whether it's a valid token.
   *
   * @return
   */
  bool Empty() {
    return token_.empty();
  }

  [[nodiscard]] std::string GetToken() {
    return token_;
  }

  [[nodiscard]] TokenType GetType() const {
    return type_;
  }

  [[nodiscard]] int GetLine() const {
    return line_;
  }

  [[nodiscard]] int GetColumn() const {
    return column_;
  }

  void SetLine(int line) {
    line_ = line;
  }

  void SetColumn(int column) {
    column_ = column;
  }

  void SetToken(const std::string &token) {
    token_ = token;
  }

 private:
  std::string token_;  // the matched string
  TokenType type_;  // terminal symbol type
  // record token location in the source file
  int line_;
  int column_;
};
}

#endif // CCOMPILER_TOKEN_H