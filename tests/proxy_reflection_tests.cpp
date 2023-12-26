// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <memory>
#include <typeinfo>
#include "proxy.h"

namespace {

template <class F>
concept ReflectionApplicable = requires(pro::proxy<F> p) {
  { p.reflect() };
};

class RttiReflection {
 public:
  template <class P>
  constexpr explicit RttiReflection(std::in_place_type_t<P>)
      : type_(typeid(P)) {}

  const char* GetName() const noexcept { return type_.name(); }

 private:
  const std::type_info& type_;
};

struct TraitsReflection {
 public:
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

PRO_DEF_FACADE(DefaultFacade);
static_assert(!ReflectionApplicable<DefaultFacade>);

PRO_DEF_FACADE(TestRttiFacade, PRO_MAKE_DISPATCH_PACK(), pro::relocatable_ptr_constraints, RttiReflection);
static_assert(ReflectionApplicable<TestRttiFacade>);

PRO_DEF_FACADE(TestTraitsFacade, PRO_MAKE_DISPATCH_PACK(), pro::relocatable_ptr_constraints, TraitsReflection);
static_assert(ReflectionApplicable<TestTraitsFacade>);

}  // namespace

TEST(ProxyReflectionTests, TestRtti_RawPtr) {
  int foo = 123;
  pro::proxy<TestRttiFacade> p = &foo;
  ASSERT_EQ(p.reflect().GetName(), typeid(int*).name());
}

TEST(ProxyReflectionTests, TestRtti_FancyPtr) {
  pro::proxy<TestRttiFacade> p = std::make_unique<double>(1.23);
  ASSERT_EQ(p.reflect().GetName(), typeid(std::unique_ptr<double>).name());
}

TEST(ProxyReflectionTests, TestTraits_RawPtr) {
  int foo = 123;
  pro::proxy<TestTraitsFacade> p = &foo;
  ASSERT_EQ(p.reflect().is_default_constructible_, true);
  ASSERT_EQ(p.reflect().is_copy_constructible_, true);
  ASSERT_EQ(p.reflect().is_nothrow_move_constructible_, true);
  ASSERT_EQ(p.reflect().is_nothrow_destructible_, true);
  ASSERT_EQ(p.reflect().is_trivial_, true);
}

TEST(ProxyReflectionTests, TestTraits_FancyPtr) {
  pro::proxy<TestTraitsFacade> p = std::make_unique<double>(1.23);
  ASSERT_EQ(p.reflect().is_default_constructible_, true);
  ASSERT_EQ(p.reflect().is_copy_constructible_, false);
  ASSERT_EQ(p.reflect().is_nothrow_move_constructible_, true);
  ASSERT_EQ(p.reflect().is_nothrow_destructible_, true);
  ASSERT_EQ(p.reflect().is_trivial_, false);
}
