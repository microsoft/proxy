// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "utils.h"
#include <gtest/gtest.h>
#include <proxy/proxy.h>

namespace proxy_view_tests_details {

struct TestFacade
    : pro::facade_builder                                              //
      ::add_convention<pro::operator_dispatch<"+=">, void(int)>        //
      ::add_convention<utils::spec::FreeToString, std::string() const> //
      ::add_skill<pro::skills::as_view>                                //
      ::build {};

template <class T>
concept SupportsIntPlusEqual = requires(T a, int b) { a += b; };
template <class T>
concept SupportsToString = requires(T a) {
  { ToString(a) } -> std::same_as<std::string>;
};

static_assert(!std::is_copy_constructible_v<pro::proxy<TestFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<TestFacade>>);
static_assert(
    SupportsIntPlusEqual<decltype(*std::declval<pro::proxy<TestFacade>>())>);
static_assert(
    SupportsToString<decltype(*std::declval<pro::proxy<TestFacade>>())>);
static_assert(sizeof(pro::proxy<TestFacade>) == 3 * sizeof(void*));

static_assert(
    std::is_trivially_copy_constructible_v<pro::proxy_view<TestFacade>>);
static_assert(std::is_trivially_destructible_v<pro::proxy_view<TestFacade>>);
static_assert(SupportsIntPlusEqual<
              decltype(*std::declval<pro::proxy_view<TestFacade>>())>);
static_assert(
    SupportsToString<decltype(*std::declval<pro::proxy_view<TestFacade>>())>);
static_assert(sizeof(pro::proxy_view<TestFacade>) == 3 * sizeof(void*));

static_assert(std::is_nothrow_convertible_v<pro::proxy<TestFacade>&,
                                            pro::proxy_view<TestFacade>>);
static_assert(!std::is_convertible_v<pro::proxy<TestFacade>,
                                     pro::proxy_view<TestFacade>>);

template <class F>
using AreEqualOverload = bool(const pro::proxy_indirect_accessor<F>& rhs,
                              double eps) const;

template <class T, pro::facade F>
bool AreEqualImpl(const T& lhs, const pro::proxy_indirect_accessor<F>& rhs,
                  double eps) {
  return lhs.AreEqual(proxy_cast<const T&>(rhs), eps);
}

PRO_DEF_FREE_AS_MEM_DISPATCH(MemAreEqual, AreEqualImpl, AreEqual);

struct EqualableQuantity
    : pro::facade_builder            //
      ::add_skill<pro::skills::rtti> // for proxy_cast
      ::add_convention<MemAreEqual,
                       pro::facade_aware_overload_t<AreEqualOverload>> //
      ::build {};

class Point_2 {
public:
  Point_2(double x, double y) : x_(x), y_(y) {}
  Point_2(const Point_2&) = default;
  Point_2& operator=(const Point_2&) = default;
  bool AreEqual(const Point_2& rhs, double eps) const {
    return std::abs(x_ - rhs.x_) < eps && std::abs(y_ - rhs.y_) < eps;
  }

private:
  double x_, y_;
};

} // namespace proxy_view_tests_details

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
  pro::proxy<details::TestFacade> p1 =
      pro::make_proxy<details::TestFacade>(123);
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

TEST(ProxyViewTests, TestOverloadShadowing) {
  struct TestFacade
      : pro::facade_builder                                                //
        ::add_convention<pro::operator_dispatch<"()">, int(), int() const> //
        ::add_skill<pro::skills::as_view>                                  //
        ::build {};
  struct TestImpl {
    int operator()() { return 0; }
    int operator()() const { return 1; }
  };
  pro::proxy<TestFacade> p1 = pro::make_proxy<TestFacade, TestImpl>();
  pro::proxy_view<TestFacade> p2 = p1;
  ASSERT_EQ((*p1)(), 0);
  ASSERT_EQ((std::as_const(*p1))(), 1);
  ASSERT_EQ((*p2)(), 0);
  ASSERT_EQ((std::as_const(*p2))(), 1);
}

TEST(ProxyViewTests, TestSubstitution_FromNull) {
  struct TestFacade1 : pro::facade_builder::build {};
  struct TestFacade2 : pro::facade_builder             //
                       ::add_facade<TestFacade1, true> // Supports substitution
                       ::add_skill<pro::skills::as_view> //
                       ::build {};
  pro::proxy<TestFacade2> p1;
  pro::proxy_view<TestFacade2> p2 = p1;
  pro::proxy_view<TestFacade1> p3 = p2;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
  ASSERT_FALSE(p3.has_value());
}

TEST(ProxyViewTests, TestSubstitution_FromValue) {
  struct TestFacade1
      : pro::facade_builder                                              //
        ::add_convention<utils::spec::FreeToString, std::string() const> //
        ::build {};
  struct TestFacade2 : pro::facade_builder                               //
                       ::support_copy<pro::constraint_level::nontrivial> //
                       ::add_facade<TestFacade1, true> // Supports substitution
                       ::add_skill<pro::skills::as_view> //
                       ::build {};
  pro::proxy<TestFacade2> p1 = pro::make_proxy<TestFacade2>(123);
  pro::proxy_view<TestFacade2> p2 = p1;
  pro::proxy_view<TestFacade1> p3 = p2;
  ASSERT_EQ(ToString(*p1), "123");
  ASSERT_EQ(ToString(*p2), "123");
  ASSERT_EQ(ToString(*p3), "123");
}

TEST(ProxyViewTests, TestFacadeAware) {
  details::Point_2 v1{1, 1};
  details::Point_2 v2{1, 0};
  details::Point_2 v3{1.0000001, 1.0000001};
  pro::proxy_view<details::EqualableQuantity> p1 =
      pro::make_proxy_view<details::EqualableQuantity>(v1);
  pro::proxy_view<details::EqualableQuantity> p2 =
      pro::make_proxy_view<details::EqualableQuantity>(v2);
  pro::proxy_view<details::EqualableQuantity> p3 =
      pro::make_proxy_view<details::EqualableQuantity>(v3);
  ASSERT_TRUE(p1->AreEqual(*p1, 1e-6));
  ASSERT_FALSE(p1->AreEqual(*p2, 1e-6));
  ASSERT_TRUE(p1->AreEqual(*p3, 1e-6));
}
