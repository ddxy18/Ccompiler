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