//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_TYPE_H
#define CCOMPILER_TYPE_H

namespace CCompiler {

class Type {
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
};

class QualType : public Type {
  enum class Qualifier {
    kConst,
    kRestrict,
    kVolatile,
    k_Atomic,
    kEmpty
  };
};

class PointerType : public QualType {
};

class VoidType : public Type {
};

class ArrayType : public QualType {
};

class FuncType : public QualType {
};

class EnumType : public Type {
};

class StructType : public Type {
};

class UnionType : public Type {
};
}

#endif // CCOMPILER_TYPE_H