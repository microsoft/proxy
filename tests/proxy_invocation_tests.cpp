// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <algorithm>
#include <list>
#include <ranges>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include "proxy.h"

namespace {

namespace poly {

template <class... Os>
PRO_DEF_FREE_DISPATCH(Call, std::invoke, Os...);
template <class... Os>
PRO_DEF_FACADE(Callable, Call<Os...>, pro::copyable_pointer_constraints);

PRO_DEF_FREE_DISPATCH(GetSize, std::ranges::size, std::size_t());

template <class T>
PRO_DEF_FREE_DISPATCH(ForEach, std::ranges::for_each, void(pro::proxy<Callable<void(T&)>>));
template <class T>
PRO_DEF_FACADE(Iterable, PRO_MAKE_DISPATCH_PACK(ForEach<T>, GetSize));

template <class T> struct Append;
template <class T>
PRO_DEF_FACADE(Container, PRO_MAKE_DISPATCH_PACK(ForEach<T>, GetSize, Append<T>));
template <class T>
struct Append {
  using overload_types = std::tuple<pro::proxy<Container<T>>(T)>;

  template <class U>
  pro::proxy<Container<T>> operator()(U& self, T&& value) {
    self.push_back(std::move(value));
    return &self;
  }
};

}  // namespace poly


template <class F, class D, class... Args>
concept InvocableWithDispatch = requires(pro::proxy<F> p, Args... args)
    { { p.template invoke<D>(std::forward<Args>(args)...) }; };
template <class F, class... Args>
concept InvocableWithoutDispatch = std::is_invocable_v<pro::proxy<F>, Args...>;

// Static assertions for a facade of a single dispatch
static_assert(InvocableWithDispatch<poly::Callable<int(double)>, poly::Call<int(double)>, double>);
static_assert(!InvocableWithDispatch<poly::Callable<int(double)>, poly::Call<int(double)>, std::nullptr_t>);  // Wrong arguments
static_assert(!InvocableWithoutDispatch<poly::Callable<int(double)>, std::nullptr_t>);  // Wrong arguments
static_assert(!InvocableWithDispatch<poly::Callable<int(double)>, int(double), double>);  // Wrong dispatch
static_assert(InvocableWithoutDispatch<poly::Callable<int(double)>, float>);  // Invoking without specifying a dispatch

// Static assertions for a facade of multiple dispatches
static_assert(InvocableWithDispatch<poly::Iterable<int>, poly::GetSize>);
static_assert(!InvocableWithDispatch<poly::Iterable<int>, poly::ForEach<int>, pro::proxy<poly::Callable<void(double&)>>>);  // Wrong arguments
static_assert(!InvocableWithDispatch<poly::Iterable<int>, poly::Append<int>>);  // Wrong dispatch
static_assert(!InvocableWithoutDispatch<poly::Iterable<int>>);  // Invoking without specifying a dispatch

template <class... Args>
std::vector<std::type_index> GetTypeIndices()
    { return {std::type_index{typeid(Args)}...}; }

}  // namespace

TEST(ProxyInvocationTests, TestArgumentForwarding) {
  std::string arg1 = "My string";
  std::vector<int> arg2 = { 1, 2, 3 };
  std::vector<int> arg2_copy = arg2;
  std::string arg1_received;
  std::vector<int> arg2_received;
  int expected_result = 456;
  auto f = [&](std::string&& s, std::vector<int>&& v) -> int {
    arg1_received = std::move(s);
    arg2_received = std::move(v);
    return expected_result;
  };
  pro::proxy<poly::Callable<int(std::string, std::vector<int>)>> p = &f;
  int result = p.invoke(arg1, std::move(arg2));
  ASSERT_TRUE(p.has_value());
  ASSERT_EQ(arg1_received, arg1);
  ASSERT_TRUE(arg2.empty());
  ASSERT_EQ(arg2_received, arg2_copy);
  ASSERT_EQ(result, expected_result);
}

TEST(ProxyInvocationTests, TestThrow) {
  const char* expected_error_message = "My exception";
  auto f = [&] { throw std::runtime_error{ expected_error_message }; };
  bool exception_thrown = false;
  pro::proxy<poly::Callable<void()>> p = &f;
  try {
    p.invoke();
  } catch (const std::runtime_error& e) {
    exception_thrown = true;
    ASSERT_STREQ(e.what(), expected_error_message);
  }
  ASSERT_TRUE(exception_thrown);
  ASSERT_TRUE(p.has_value());
}

TEST(ProxyInvocationTests, TestMultipleDispatches_Unique) {
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<poly::Iterable<int>> p = &l;
  ASSERT_EQ(p.invoke<poly::GetSize>(), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  p.invoke<poly::ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 6);
}

TEST(ProxyInvocationTests, TestMultipleDispatches_Duplicated) {
  using SomeCombination = std::tuple<poly::ForEach<int>, std::tuple<poly::GetSize, poly::ForEach<int>>>;
  PRO_DEF_FACADE(DuplicatedIterable, PRO_MAKE_DISPATCH_PACK(poly::ForEach<int>, SomeCombination, poly::ForEach<int>, poly::GetSize, poly::GetSize));
  static_assert(sizeof(pro::details::facade_traits<DuplicatedIterable>::meta_type) ==
      sizeof(pro::details::facade_traits<poly::Iterable<int>>::meta_type));
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<DuplicatedIterable> p = &l;
  ASSERT_EQ(p.invoke<poly::GetSize>(), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  p.invoke<poly::ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 6);
}

TEST(ProxyInvocationTests, TestRecursiveDefinition) {
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<poly::Container<int>> p = &l;
  ASSERT_EQ(p.invoke<poly::GetSize>(), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  p.invoke<poly::ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 6);
  p.invoke<poly::Append<int>>(4).invoke<poly::Append<int>>(5).invoke<poly::Append<int>>(6);
  ASSERT_EQ(p.invoke<poly::GetSize>(), 6);
  sum = 0;
  p.invoke<poly::ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 21);
}

TEST(ProxyInvocationTests, TestOverloadResolution) {
  struct TestFacade : poly::Callable<void(int), void(double), void(char*),
      void(const char*), void(std::string, int)> {};
  std::vector<std::type_index> side_effect;
  auto p = pro::make_proxy<TestFacade>([&](auto&&... args)
      { side_effect = GetTypeIndices<std::decay_t<decltype(args)>...>(); });
  p(123);
  ASSERT_EQ(side_effect, GetTypeIndices<int>());
  p(1.23);
  ASSERT_EQ(side_effect, GetTypeIndices<double>());
  char foo[2];
  p(foo);
  ASSERT_EQ(side_effect, GetTypeIndices<char*>());
  p("lalala");
  ASSERT_EQ(side_effect, GetTypeIndices<const char*>());
  p("lalala", 0);
  ASSERT_EQ(side_effect, (GetTypeIndices<std::string, int>()));
  ASSERT_FALSE((std::is_invocable_v<decltype(p), std::vector<int>>));
}
