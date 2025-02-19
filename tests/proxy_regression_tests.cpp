// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include "proxy.h"

template <class It, class F>
bool operator==(const It& it, const pro::proxy<F>& rhs) noexcept
    requires(!std::is_same_v<It, pro::proxy<F>>) {
  return typeid(It) == proxy_typeid(rhs) && it == proxy_cast<const It&>(rhs);
}

namespace proxy_regression_tests_details {

template <class F>
using SelfComparisonOverload = bool(const pro::proxy<F>& rhs) const noexcept;

template <class T>
struct Iterator : pro::facade_builder
    ::support_direct_rtti
    ::add_direct_convention<pro::operator_dispatch<"++">, void() noexcept>
    ::add_direct_convention<pro::operator_dispatch<"!=">, pro::facade_aware_overload_t<SelfComparisonOverload>>
    ::add_convention<pro::implicit_conversion_dispatch, T&() const noexcept>
    ::build {};

}  // namespace proxy_regression_tests_details

namespace details = proxy_regression_tests_details;

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

// https://github.com/microsoft/proxy/issues/254
TEST(ProxyRegressionTests, TestProxiableSelfDependency) {
  int a[3]{1, 2, 3};
  pro::proxy<details::Iterator<int>> p = a;
  ASSERT_EQ(*p, 1);
  ++p;
  ASSERT_EQ(*p, 2);
}
