//
// Created by dxy on 2020/11/13.
//

#include "gtest/gtest.h"
#include "ast/list_util.h"

using namespace CCompiler;
using namespace std;

TEST(ListUtil, ContainPointer) {
  list<int *> l1{new int(1), new int(2)};
  list<int *> l2{new int(1), new int(2)};

  EXPECT_TRUE(Equal(l1, l2));
}

TEST(ListUtil, DifferentObject) {
  list<int *> l1{new int(2), new int(1)};
  list<int *> l2{new int(1), new int(2)};

  EXPECT_FALSE(Equal(l1, l2));
}

TEST(ListUtil, DifferentSize) {
  list<int *> l1{new int(1)};
  list<int *> l2{new int(1), new int(2)};

  EXPECT_FALSE(Equal(l1, l2));
}

TEST(ListUtil, DifferentType) {
  class Base {
   public:
    explicit Base(int i) : i_(i) {}

    virtual bool operator==(const Base &rhs) const {
      return i_ == rhs.i_;
    }

    virtual bool operator!=(const Base &rhs) const {
      return !(rhs == *this);
    }

   private:
    int i_;
  };
  class Derived : public Base {
   public:
    Derived(int i, int j) : Base(i), j_(j) {}

    bool operator==(const Derived &rhs) const {
      return Base::operator==(rhs) && j_ == rhs.j_;
    }

    bool operator!=(const Derived &rhs) const {
      return !(rhs == *this);
    }

   private:
    int j_;
  };

  list<Base *> l1{new Base(1), new Derived(1, 2)};
  list<Base *> l2{new Derived(1, 2), new Base(1)};

  EXPECT_FALSE(Equal(l1, l2));
}