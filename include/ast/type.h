//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_TYPE_H
#define CCOMPILER_TYPE_H

#include <list>
#include <string>
#include <utility>

namespace CCompiler {
class Identifier;

/**
 * Use a bit to determine whether a given qualifier exists. This means that
 * repeated qualifiers will be ignored.
 */
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

  bool operator==(const Type &rhs) const {
    return true;
  }

  bool operator!=(const Type &rhs) const {
    return !(rhs == *this);
  }

  virtual bool Equal(const Type *rhs) const = 0;
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

  QualType(unsigned int specifier, unsigned char qualifier)
          : specifier_(specifier),
            qualifier_(qualifier) {}

  void AddQualifier(Qualifier qualifier) {
    qualifier_ |= qualifier;
  }

  void AddSpecifier(Specifier specifier) {
    specifier_ |= specifier;
  }

  bool operator==(const QualType &rhs) const {
    return specifier_ == rhs.specifier_ && qualifier_ == rhs.qualifier_;
  }

  bool operator!=(const QualType &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Type *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(QualType) &&
           *this == *dynamic_cast<const QualType *>(rhs);
  }

 private:
  unsigned int specifier_;  //!< must > 0
  unsigned char qualifier_;
};

class DerivedType : public Type {
 public:
  explicit DerivedType(Type *derived) : derived_(derived) {}

  bool operator==(const DerivedType &rhs) const {
    if (derived_ == nullptr) {
      return rhs.derived_ == nullptr;
    }
    return derived_->Equal(rhs.derived_);
  }

  bool operator!=(const DerivedType &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Type *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(DerivedType) &&
           *this == *dynamic_cast<const DerivedType *>(rhs);
  }

 private:
  Type *derived_;  //!< DerivedType is derived from it
};

class PointerType : public DerivedType {
 public:
  PointerType(Type *derived, Qualifier qualifier)
          : DerivedType(derived),
            qualifier_(qualifier) {}

  bool operator==(const PointerType &rhs) const {
    return DerivedType::operator==(rhs) && qualifier_ == rhs.qualifier_;
  }

  bool operator!=(const PointerType &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Type *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(PointerType) &&
           *this == *dynamic_cast<const PointerType *>(rhs);
  }

 private:
  Qualifier qualifier_;
};

class ArrayType : public DerivedType {
 public:
  explicit ArrayType(Type *derived, int length)
          : DerivedType(derived),
            length_(length) {}

  bool operator==(const ArrayType &rhs) const {
    return DerivedType::operator==(rhs) && length_ == rhs.length_;
  }

  bool operator!=(const ArrayType &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Type *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(ArrayType) &&
           *this == *dynamic_cast<const ArrayType *>(rhs);
  }

 private:
  //!< element type is stored in derived_

  int length_;  //!< use -1 to represent incomplete type
};

// TODO(dxy): for declaring function pointers and type checking when calling
//  a function
class FuncType : public DerivedType {
 public:
  using ParamList = std::list<Type *>;

  bool operator==(const FuncType &rhs) const;

  bool operator!=(const FuncType &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Type *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(FuncType) &&
           *this == *dynamic_cast<const FuncType *>(rhs);
  }

 private:
  //!< return type is stored in derived_

  ParamList params_;  //!< type information for parameters in sequence
};

class TypeDeclType : public Type {
  friend
  bool operator==(const TypeDeclType &lhs, const TypeDeclType &rhs);

 public:
  explicit TypeDeclType(std::string ident) : ident_(std::move(ident)) {}

  [[nodiscard]] const std::string &GetIdent() const {
    return ident_;
  }

  bool operator!=(const TypeDeclType &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Type *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(TypeDeclType) &&
           *this == *dynamic_cast<const TypeDeclType *>(rhs);
  }

 private:
  std::string ident_;
};

bool operator==(const TypeDeclType &lhs, const TypeDeclType &rhs);

//!< members in struct and union
using MemberList = std::list<Identifier *>;

class StructUnionType : public TypeDeclType {
 public:
  explicit StructUnionType(bool flag, std::string tag = "")
          : TypeDeclType(std::move(tag)),
            flag_(flag) {}

  void AddMember(Identifier *ident) {
    members_.push_back(ident);
  }

  bool operator==(const StructUnionType &rhs) const;

  bool operator!=(const StructUnionType &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Type *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(StructUnionType) &&
           *this == *dynamic_cast<const StructUnionType *>(rhs);
  }

 private:
  bool flag_;  //!< true--struct, false--union
  MemberList members_;
};

class EnumType : public TypeDeclType {
 public:
  struct Enumerator {
    std::string ident_;
    int value_{-1};  //!< -1 represents no appointed value.

    bool operator==(const Enumerator &rhs) const {
      return ident_ == rhs.ident_ && value_ == rhs.value_;
    }

    bool operator!=(const Enumerator &rhs) const {
      return !(rhs == *this);
    }
  };

  explicit EnumType(std::string tag) : TypeDeclType(std::move(tag)) {}

  void AddEnumerator(const Enumerator &enumerator) {
    enumerators_.push_back(enumerator);
  }

  bool operator==(const EnumType &rhs) const {
    return TypeDeclType::operator==(rhs) && enumerators_ == rhs.enumerators_;
  }

  bool operator!=(const EnumType &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Type *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(EnumType) &&
           *this == *dynamic_cast<const EnumType *>(rhs);
  }

 private:
  std::list<Enumerator> enumerators_;
};

// TODO(dxy):
class TypeDef : public TypeDeclType {
 public:
  bool operator==(const TypeDef &rhs) const {
    if (type_ == nullptr) {
      return TypeDeclType::operator==(rhs) && rhs.type_ == nullptr;
    }
    return TypeDeclType::operator==(rhs) && type_->Equal(rhs.type_);
  }

  bool operator!=(const TypeDef &rhs) const {
    return !(rhs == *this);
  }

  bool Equal(const Type *rhs) const override {
    if (rhs == nullptr) {
      return false;
    }
    return typeid(*rhs) == typeid(TypeDef) &&
           *this == *dynamic_cast<const TypeDef *>(rhs);
  }

 private:
  Type *type_;
};
}

#endif // CCOMPILER_TYPE_H