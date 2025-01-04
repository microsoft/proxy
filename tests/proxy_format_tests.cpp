// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include "proxy.h"

namespace proxy_format_tests_details {

struct TestFacade : pro::facade_builder
    ::support_format
    ::support_wformat
    ::build {};

}  // namespace proxy_format_tests_details

namespace details = proxy_format_tests_details;

TEST(ProxyFormatTests, TestFormat_Null) {
  pro::proxy<details::TestFacade> p;
  bool exception_thrown = false;
  try {
    std::ignore = std::format("{}", *p);
  } catch (const std::format_error&) {
    exception_thrown = true;
  }
  ASSERT_TRUE(exception_thrown);
}

TEST(ProxyFormatTests, TestFormat_Value) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  ASSERT_EQ(std::format("{}", *p), "123");
  ASSERT_EQ(std::format("{:*<6}", *p), "123***");
}

TEST(ProxyFormatTests, TestWformat_Null) {
  pro::proxy<details::TestFacade> p;
  bool exception_thrown = false;
  try {
    std::ignore = std::format(L"{}", *p);
  } catch (const std::format_error&) {
    exception_thrown = true;
  }
  ASSERT_TRUE(exception_thrown);
}

TEST(ProxyFormatTests, TestWformat_Value) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  ASSERT_EQ(std::format(L"{}", *p), L"123");
  ASSERT_EQ(std::format(L"{:*<6}", *p), L"123***");
}
