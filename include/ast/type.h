//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_TYPE_H
#define CCOMPILER_TYPE_H

#include <list>
#include <memory>

namespace CCompiler {
class Identifier;

class Type {
 private:
  enum class Specifier {
    kChar,
    kShort,
    kInt,
    kLong,
    kFloat,
    kDouble,
    kSigned,
    kUnsigned,
    k_Bool,
    k_Complex,
    kDerived  // all other types derived from Type
  };
  Specifier specifier_;
};

class QualType : public Type {
 private:
  enum class Qualifier {
    kConst,
    kRestrict,
    kVolatile,
    k_Atomic,
    kEmpty
  };
  Qualifier qualifier_;
};

class DerivedType : public Type {
 private:
  std::unique_ptr<QualType> derived_;
};

class PointerType : public DerivedType {
};

class VoidType : public Type {
};

class ArrayType : public DerivedType {
 private:
  int length_;
  // element type is stored in derived_
};

class FuncType : public DerivedType {
 private:
  using ParamList = std::list<std::unique_ptr<Type>>;
  ParamList params_;  // type information for parameters in sequence
  // return type is stored in derived_
};

// members in struct and union
using MemberList = std::list<std::unique_ptr<Identifier>>;

class StructType : public DerivedType {
 private:
  MemberList members_;
};

class UnionType : public DerivedType {
 private:
  MemberList members_;
};
}

#endif // CCOMPILER_TYPE_H