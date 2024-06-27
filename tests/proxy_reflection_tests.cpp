// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <memory>
#include <typeinfo>
#include "proxy.h"

namespace {

class RttiReflection {
 public:
  static constexpr bool is_direct = false;

  template <class T>
  constexpr explicit RttiReflection(std::in_place_type_t<T>)
      : type_(typeid(T)) {}

  const char* GetName() const noexcept { return type_.name(); }

 private:
  const std::type_info& type_;
};

struct TraitsReflection {
 public:
  static constexpr bool is_direct = true;

  template <class P>
  constexpr explicit TraitsReflection(std::in_place_type_t<P>)
      : is_default_constructible_(std::is_default_constructible_v<P>),
        is_copy_constructible_(std::is_copy_constructible_v<P>),
        is_nothrow_move_constructible_(std::is_nothrow_move_constructible_v<P>),
        is_nothrow_destructible_(std::is_nothrow_destructible_v<P>),
        is_trivial_(std::is_trivial_v<P>) {}

  bool is_default_constructible_;
  bool is_copy_constructible_;
  bool is_nothrow_move_constructible_;
  bool is_nothrow_destructible_;
  bool is_trivial_;
};

struct TestRttiFacade : pro::facade_builder
    ::add_reflection<RttiReflection>
    ::build {};

struct TestTraitsFacade : pro::facade_builder
    ::add_reflection<TraitsReflection>
    ::build {};

}  // namespace

TEST(ProxyReflectionTests, TestRtti_RawPtr) {
  int foo = 123;
  pro::proxy<TestRttiFacade> p = &foo;
  ASSERT_STREQ(pro::proxy_reflect<RttiReflection>(p).GetName(), typeid(int).name());
}

TEST(ProxyReflectionTests, TestRtti_FancyPtr) {
  pro::proxy<TestRttiFacade> p = std::make_unique<double>(1.23);
  ASSERT_STREQ(pro::proxy_reflect<RttiReflection>(p).GetName(), typeid(double).name());
}

TEST(ProxyReflectionTests, TestTraits_RawPtr) {
  int foo = 123;
  pro::proxy<TestTraitsFacade> p = &foo;
  ASSERT_EQ(pro::proxy_reflect<TraitsReflection>(p).is_default_constructible_, true);
  ASSERT_EQ(pro::proxy_reflect<TraitsReflection>(p).is_copy_constructible_, true);
  ASSERT_EQ(pro::proxy_reflect<TraitsReflection>(p).is_nothrow_move_constructible_, true);
  ASSERT_EQ(pro::proxy_reflect<TraitsReflection>(p).is_nothrow_destructible_, true);
  ASSERT_EQ(pro::proxy_reflect<TraitsReflection>(p).is_trivial_, true);
}

TEST(ProxyReflectionTests, TestTraits_FancyPtr) {
  pro::proxy<TestTraitsFacade> p = std::make_unique<double>(1.23);
  ASSERT_EQ(pro::proxy_reflect<TraitsReflection>(p).is_default_constructible_, true);
  ASSERT_EQ(pro::proxy_reflect<TraitsReflection>(p).is_copy_constructible_, false);
  ASSERT_EQ(pro::proxy_reflect<TraitsReflection>(p).is_nothrow_move_constructible_, true);
  ASSERT_EQ(pro::proxy_reflect<TraitsReflection>(p).is_nothrow_destructible_, true);
  ASSERT_EQ(pro::proxy_reflect<TraitsReflection>(p).is_trivial_, false);
}
