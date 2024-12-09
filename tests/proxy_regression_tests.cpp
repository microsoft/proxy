// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include "proxy.h"

// https://github.com/microsoft/proxy/issues/213
TEST(ProxyRegressionTests, TestUnexpectedCompilerWarning) {
  struct MyTrivialFacade : pro::facade_builder
      ::add_convention<pro::operator_dispatch<"()">, void(), void() const>
      ::support_copy<pro::constraint_level::trivial>
      ::support_relocation<pro::constraint_level::trivial>
      ::support_destruction<pro::constraint_level::trivial>
      ::build {};
  int side_effect = 0;
  pro::proxy<MyTrivialFacade> p = pro::make_proxy<MyTrivialFacade>([&] { side_effect = 1; });
  (*p)();
  EXPECT_EQ(side_effect, 1);
}
