//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_TYPE_H
#define CCOMPILER_TYPE_H

#include <list>

namespace CCompiler {
class Identifier;

enum Qualifier {
  kEmpty = 0b0,
  kConst = 0b1,
  kRestrict = 0b10,
  kVolatile = 0b100,
  k_Atomic = 0b1000
};

class Type {
 public:
  virtual ~Type() = default;
};

class QualType : public Type {
 public:
  enum Specifier {
    kChar = 0b1,
    kShort = 0b10,
    kInt = 0b100,
    kLong = 0b1000,
    kFloat = 0b10000,
    kDouble = 0b100000,
    kSigned = 0b1000000,
    kUnsigned = 0b10000000,
    k_Bool = 0b100000000,
    k_Complex = 0b1000000000,
    kVoid = 0b10000000000
  };

  QualType() : specifier_(0), qualifier_(0) {}

  void AddQualifier(Qualifier qualifier) {
    qualifier_ |= qualifier;
  }

  void AddSpecifier(Specifier specifier) {
    specifier_ |= specifier;
  }

 private:
  unsigned int specifier_;
  unsigned char qualifier_;
};

class DerivedType : public Type {
 public:
  explicit DerivedType(Type *derived) : derived_(derived) {}

 private:
  Type *derived_;
};

class PointerType : public DerivedType {
 public:
  PointerType(Type *derived, Qualifier qualifier)
          : DerivedType(derived),
            qualifier_(qualifier) {}

 private:
  Qualifier qualifier_;
};

class ArrayType : public DerivedType {
 public:
  explicit ArrayType(Type *derived, int length)
          : DerivedType(derived),
            length_(length) {}

 private:
  int length_;
  // element type is stored in derived_
};

//
class FuncType : public DerivedType {
 private:
  using ParamList = std::list<Type *>;
  ParamList params_;  // type information for parameters in sequence
  // return type is stored in derived_
};

// members in struct and union
using MemberList = std::list<Identifier *>;

class StructUnionType : public Type {
 public:
  explicit StructUnionType(bool flag, std::string tag = "")
          : flag_(flag),
            tag_(std::move(tag)) {}

  void AddMember(Identifier *ident) {
    members_.push_back(ident);
  }

 private:
  bool flag_;  // true--struct, false--union
  std::string tag_;
  MemberList members_;
};

class EnumType : public Type {
 public:
  struct Enumerator {
    std::string ident_;
    int value_{-1};  // -1 represents no appointed value.
  };

  explicit EnumType(std::string tag) : tag_(std::move(tag)) {}

  void AddEnumerator(const Enumerator &enumerator) {
    enumerators_.push_back(enumerator);
  }

 private:
  std::string tag_;
  std::list<Enumerator> enumerators_;
};
}

#endif // CCOMPILER_TYPE_H