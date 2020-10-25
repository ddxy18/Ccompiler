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

class PointerType : public Type {
};

class VoidType : public Type {
};

class ArrayType : public Type {
};

class FuncType : public Type {
};

class EnumType : public Type {
};

class StructType : public Type {
};

class UnionType : public Type {
};
}

#endif // CCOMPILER_TYPE_H