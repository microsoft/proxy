#include <gtest/gtest.h>
#include <iostream>

import proxy.v4;
import foo;
import foo_impl;

auto user(pro::proxy<Foo> p) { return p->GetFoo(); }

TEST(ProxyModuleSupportTests, TestBasic) {
  MyFoo foo;
  ASSERT_EQ(user(&foo), 42);
}
