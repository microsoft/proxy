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

template <class... Os>
DEFINE_MEMBER_DISPATCH(Call, operator(), Os...);
template <class... Os>
DEFINE_COPYABLE_FACADE(CallableFacade, Call<Os...>);

DEFINE_FREE_DISPATCH(GetSize, std::ranges::size, std::size_t());

template <class T>
DEFINE_FREE_DISPATCH(ForEach, std::ranges::for_each, void(pro::proxy<CallableFacade<void(T&)>>));
template <class T>
DEFINE_FACADE(IterableFacade, ForEach<T>, GetSize);

template <class T> struct Append;
template <class T>
DEFINE_FACADE(ContainerFacade, ForEach<T>, GetSize, Append<T>);
template <class T>
struct Append : pro::dispatch<pro::proxy<ContainerFacade<T>>(T)> {
  template <class U>
  pro::proxy<ContainerFacade<T>> operator()(U& self, T&& value) {
    self.push_back(std::move(value));
    return &self;
  }
};

template <class F, class D, class... Args>
concept InvocableWithDispatch = requires(pro::proxy<F> p, Args... args)
    { { p.template invoke<D>(std::forward<Args>(args)...) }; };
template <class F, class... Args>
concept InvocableWithoutDispatch = std::is_invocable_v<pro::proxy<F>, Args...>;

// Static assertions for a facade of a single dispatch
static_assert(InvocableWithDispatch<CallableFacade<int(double)>, Call<int(double)>, double>);
static_assert(!InvocableWithDispatch<CallableFacade<int(double)>, Call<int(double)>, std::nullptr_t>);  // Wrong arguments
static_assert(!InvocableWithoutDispatch<CallableFacade<int(double)>, std::nullptr_t>);  // Wrong arguments
static_assert(!InvocableWithDispatch<CallableFacade<int(double)>, int(double), double>);  // Wrong dispatch
static_assert(InvocableWithoutDispatch<CallableFacade<int(double)>, float>);  // Invoking without specifying a dispatch

// Static assertions for a facade of multiple dispatches
static_assert(InvocableWithDispatch<IterableFacade<int>, GetSize>);
static_assert(!InvocableWithDispatch<IterableFacade<int>, ForEach<int>, pro::proxy<CallableFacade<void(double&)>>>);  // Wrong arguments
static_assert(!InvocableWithDispatch<IterableFacade<int>, Append<int>>);  // Wrong dispatch
static_assert(!InvocableWithoutDispatch<IterableFacade<int>>);  // Invoking without specifying a dispatch

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
  pro::proxy<CallableFacade<int(std::string, std::vector<int>)>> p = &f;
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
  pro::proxy<CallableFacade<void()>> p = &f;
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
  pro::proxy<IterableFacade<int>> p = &l;
  ASSERT_EQ(p.invoke<GetSize>(), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  p.invoke<ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 6);
}

TEST(ProxyInvocationTests, TestMultipleDispatches_Duplicated) {
  using SomeCombination = std::tuple<ForEach<int>, std::tuple<GetSize, ForEach<int>>>;
  struct DuplicatedIterableFacade : pro::facade<
      ForEach<int>, SomeCombination, ForEach<int>, GetSize, GetSize> {};
  static_assert(sizeof(pro::details::facade_traits<DuplicatedIterableFacade>::meta_type) ==
      sizeof(pro::details::facade_traits<IterableFacade<int>>::meta_type));
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<DuplicatedIterableFacade> p = &l;
  ASSERT_EQ(p.invoke<GetSize>(), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  p.invoke<ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 6);
}

TEST(ProxyInvocationTests, TestRecursiveDefinition) {
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<ContainerFacade<int>> p = &l;
  ASSERT_EQ(p.invoke<GetSize>(), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  p.invoke<ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 6);
  p.invoke<Append<int>>(4).invoke<Append<int>>(5).invoke<Append<int>>(6);
  ASSERT_EQ(p.invoke<GetSize>(), 6);
  sum = 0;
  p.invoke<ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 21);
}

TEST(ProxyInvocationTests, TestOverloadResolution) {
  struct TestFacade : CallableFacade<void(int), void(double), void(char*),
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
