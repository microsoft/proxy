// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include "proxy.h"
#include "utils.h"

namespace proxy_view_tests_details {

struct TestFacade : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"+=">, void(int)>
    ::add_convention<utils::spec::FreeToString, std::string(), std::string() const>
    ::add_view<TestFacade>
    ::add_view<const TestFacade>
    ::build {};

template <class T>
concept SupportsIntPlusEqual = requires(T a, int b) {
  a += b;
};
template <class T>
concept SupportsToString = requires(T a) {
  { ToString(a) } -> std::same_as<std::string>;
};

static_assert(!std::is_copy_constructible_v<pro::proxy<TestFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<TestFacade>>);
static_assert(SupportsIntPlusEqual<decltype(*std::declval<pro::proxy<TestFacade>>())>);
static_assert(SupportsToString<decltype(*std::declval<pro::proxy<TestFacade>>())>);
static_assert(sizeof(pro::proxy<TestFacade>) == 3 * sizeof(void*));

static_assert(std::is_trivially_copy_constructible_v<pro::proxy_view<TestFacade>>);
static_assert(std::is_trivially_destructible_v<pro::proxy_view<TestFacade>>);
static_assert(SupportsIntPlusEqual<decltype(*std::declval<pro::proxy_view<TestFacade>>())>);
static_assert(SupportsToString<decltype(*std::declval<pro::proxy_view<TestFacade>>())>);
static_assert(sizeof(pro::proxy_view<TestFacade>) == 3 * sizeof(void*));

static_assert(std::is_trivially_copy_constructible_v<pro::proxy_view<const TestFacade>>);
static_assert(std::is_trivially_destructible_v<pro::proxy_view<const TestFacade>>);
static_assert(!SupportsIntPlusEqual<decltype(*std::declval<pro::proxy_view<const TestFacade>>())>);
static_assert(SupportsToString<decltype(*std::declval<pro::proxy_view<const TestFacade>>())>);
static_assert(sizeof(pro::proxy_view<const TestFacade>) == 2 * sizeof(void*));

}  // namespace proxy_view_tests_details

namespace details = proxy_view_tests_details;

TEST(ProxyViewTests, TestViewOfNull) {
  pro::proxy<details::TestFacade> p1;
  pro::proxy_view<details::TestFacade> p2 = p1;
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyViewTests, TestViewIndependentUse) {
  int a = 123;
  pro::proxy_view<details::TestFacade> p = &a;
  *p += 3;
  ASSERT_EQ(ToString(*p), "126");
  ASSERT_EQ(a, 126);
}

TEST(ProxyViewTests, TestViewOfOwning) {
  pro::proxy<details::TestFacade> p1 = pro::make_proxy<details::TestFacade>(123);
  pro::proxy_view<details::TestFacade> p2 = p1;
  ASSERT_TRUE(p1.has_value());
  ASSERT_TRUE(p2.has_value());
  *p2 += 3;
  ASSERT_EQ(ToString(*p1), "126");
  p1.reset();
  // p2 becomes dangling
}

TEST(ProxyViewTests, TestViewOfNonOwning) {
  int a = 123;
  pro::proxy<details::TestFacade> p1 = &a;
  pro::proxy_view<details::TestFacade> p2 = p1;
  ASSERT_TRUE(p1.has_value());
  ASSERT_TRUE(p2.has_value());
  *p2 += 3;
  ASSERT_EQ(ToString(*p1), "126");
  p1.reset();
  ASSERT_EQ(ToString(*p2), "126");
  ASSERT_EQ(a, 126);
}

TEST(ProxyViewTests, TestConstViewOfNull) {
  pro::proxy<details::TestFacade> p1;
  pro::proxy_view<const details::TestFacade> p2 = std::as_const(p1);
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyViewTests, TestConstViewIndependentUse) {
  int a = 123;
  pro::proxy_view<const details::TestFacade> p = &std::as_const(a);
  a += 3;
  ASSERT_EQ(ToString(*p), "126");
}

TEST(ProxyViewTests, TestConstViewOfOwning) {
  pro::proxy<details::TestFacade> p1 = pro::make_proxy<details::TestFacade>(123);
  pro::proxy_view<const details::TestFacade> p2 = std::as_const(p1);
  ASSERT_TRUE(p1.has_value());
  ASSERT_TRUE(p2.has_value());
  *p1 += 3;
  ASSERT_EQ(ToString(*p1), "126");
  ASSERT_EQ(ToString(*p2), "126");
  p1.reset();
  // p2 becomes dangling
}

TEST(ProxyViewTests, TestConstViewOfNonOwning) {
  int a = 123;
  pro::proxy<details::TestFacade> p1 = &a;
  pro::proxy_view<const details::TestFacade> p2 = &std::as_const(a);
  ASSERT_TRUE(p1.has_value());
  ASSERT_TRUE(p2.has_value());
  *p1 += 3;
  ASSERT_EQ(ToString(*p1), "126");
  p1.reset();
  ASSERT_EQ(ToString(*p2), "126");
  ASSERT_EQ(a, 126);
}

TEST(ProxyViewTests, TestOverloadShadowing) {
  struct TestFacade : pro::facade_builder
      ::add_convention<pro::operator_dispatch<"()">, int(), int() const>
      ::add_view<TestFacade>
      ::add_view<const TestFacade>
      ::build {};
  struct TestImpl {
    int operator()() { return 0; }
    int operator()() const { return 1; }
  };
  pro::proxy<TestFacade> p1 = pro::make_proxy<TestFacade, TestImpl>();
  pro::proxy_view<TestFacade> p2 = p1;
  pro::proxy_view<const TestFacade> p3 = std::as_const(p1);
  ASSERT_EQ((*p1)(), 0);
  ASSERT_EQ((std::as_const(*p1))(), 1);
  ASSERT_EQ((*p2)(), 0);
  ASSERT_EQ((*p3)(), 1);
}

TEST(ProxyViewTests, TestUpwardConversion_FromNull) {
  struct TestFacade1 : pro::facade_builder::build {};
  struct TestFacade2 : pro::facade_builder
      ::add_facade<TestFacade1, true>  // Supports upward conversion
      ::add_view<TestFacade2>
      ::build {};
  pro::proxy<TestFacade2> p1;
  pro::proxy_view<TestFacade2> p2 = p1;
  pro::proxy_view<TestFacade1> p3 = p2;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
  ASSERT_FALSE(p3.has_value());
}

TEST(ProxyViewTests, TestUpwardConversion_FromValue) {
  struct TestFacade1 : pro::facade_builder
      ::add_convention<utils::spec::FreeToString, std::string() const>
      ::build {};
  struct TestFacade2 : pro::facade_builder
      ::support_copy<pro::constraint_level::nontrivial>
      ::add_facade<TestFacade1, true>  // Supports upward conversion
      ::add_view<TestFacade2>
      ::add_view<const TestFacade2>
      ::build {};
  pro::proxy<TestFacade2> p1 = pro::make_proxy<TestFacade2>(123);
  pro::proxy_view<TestFacade2> p2 = p1;
  pro::proxy_view<const TestFacade2> p3 = std::as_const(p1);
  pro::proxy_view<TestFacade1> p4 = p2;
  pro::proxy_view<const TestFacade1> p5 = std::as_const(p3);
  ASSERT_EQ(ToString(*p1), "123");
  ASSERT_EQ(ToString(*p3), "123");
  std::ignore = p4;
  ASSERT_EQ(ToString(*p5), "123");
}
