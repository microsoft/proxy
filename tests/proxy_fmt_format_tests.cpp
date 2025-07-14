// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>

#if defined(__NVCOMPILER)
#pragma diagnostic push
#pragma diag_suppress inline_gnu_noinline_conflict
#pragma diagnostic push
#pragma diag_suppress code_is_unreachable
#endif // defined(__NVCOMPILER)
#include <fmt/format.h>
#include <fmt/xchar.h>
#if defined(__NVCOMPILER)
#pragma diagnostic pop
#pragma diagnostic pop
#endif // defined(__NVCOMPILER)

#include <proxy/proxy.h>
#include <proxy/proxy_fmt.h>

namespace proxy_fmt_format_tests_details {

struct NonFormattable : pro::facade_builder::build {};

static_assert(
    !std::is_default_constructible_v<
        fmt::formatter<pro::proxy_indirect_accessor<NonFormattable>, char>>);
static_assert(
    !std::is_default_constructible_v<
        fmt::formatter<pro::proxy_indirect_accessor<NonFormattable>, wchar_t>>);

struct Formattable : pro::facade_builder                 //
                     ::support<pro::skills::fmt_format>  //
                     ::support<pro::skills::fmt_wformat> //
                     ::build {};

static_assert(std::is_default_constructible_v<
              fmt::formatter<pro::proxy_indirect_accessor<Formattable>, char>>);
static_assert(
    std::is_default_constructible_v<
        fmt::formatter<pro::proxy_indirect_accessor<Formattable>, wchar_t>>);

} // namespace proxy_fmt_format_tests_details

namespace details = proxy_fmt_format_tests_details;

TEST(ProxyFmtFormatTests, TestFormat) {
  int v = 123;
  pro::proxy<details::Formattable> p = &v;
  ASSERT_EQ(fmt::format("{}", *p), "123");
  ASSERT_EQ(fmt::format("{:*<6}", *p), "123***");
}

TEST(ProxyFmtFormatTests, TestWformat) {
  int v = 123;
  pro::proxy<details::Formattable> p = &v;
  ASSERT_EQ(fmt::format(L"{}", *p), L"123");
  ASSERT_EQ(fmt::format(L"{:*<6}", *p), L"123***");
}
