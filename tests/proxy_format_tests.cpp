// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <proxy/proxy.h>

namespace proxy_format_tests_details {

struct NonFormattable : pro::facade_builder::build {};

static_assert(
    !std::is_default_constructible_v<
        std::formatter<pro::proxy_indirect_accessor<NonFormattable>, char>>);
static_assert(
    !std::is_default_constructible_v<
        std::formatter<pro::proxy_indirect_accessor<NonFormattable>, wchar_t>>);

struct Formattable : pro::facade_builder             //
                     ::support<pro::skills::format>  //
                     ::support<pro::skills::wformat> //
                     ::build {};

static_assert(std::is_default_constructible_v<
              std::formatter<pro::proxy_indirect_accessor<Formattable>, char>>);
static_assert(
    std::is_default_constructible_v<
        std::formatter<pro::proxy_indirect_accessor<Formattable>, wchar_t>>);

} // namespace proxy_format_tests_details

namespace details = proxy_format_tests_details;

TEST(ProxyFormatTests, TestFormat) {
  int v = 123;
  pro::proxy<details::Formattable> p = &v;
  ASSERT_EQ(std::format("{}", *p), "123");
  ASSERT_EQ(std::format("{:*<6}", *p), "123***");
}

TEST(ProxyFormatTests, TestWformat) {
  int v = 123;
  pro::proxy<details::Formattable> p = &v;
  ASSERT_EQ(std::format(L"{}", *p), L"123");
  ASSERT_EQ(std::format(L"{:*<6}", *p), L"123***");
}
