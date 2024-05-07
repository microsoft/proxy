// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <algorithm>
#include <list>
#include <map>
#include <ranges>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include "proxy.h"
#include "utils.h"

namespace {

namespace spec {

template <class... Os>
PRO_DEF_FREE_DISPATCH(Call, std::invoke, Os...);
template <class... Os>
PRO_DEF_FACADE(Callable, Call<Os...>, pro::copyable_ptr_constraints);

template <class R>
R NotImplemented(auto&&...) { throw std::runtime_error{ "Not implemented!" }; }
template <class... Os>
PRO_DEF_FREE_DISPATCH_WITH_DEFAULT(WeakCall, std::invoke, NotImplemented<void>, Os...);
template <class... Os>
PRO_DEF_FACADE(WeakCallable, WeakCall<Os...>, pro::copyable_ptr_constraints);

PRO_DEF_FREE_DISPATCH(GetSize, std::ranges::size, std::size_t() noexcept);

template <class T>
PRO_DEF_FREE_DISPATCH(ForEach, std::ranges::for_each, void(pro::proxy<Callable<void(T&)>>));
template <class T>
PRO_DEF_FACADE(Iterable, PRO_MAKE_DISPATCH_PACK(ForEach<T>, GetSize));

template <class T> struct Append;
template <class T>
PRO_DEF_FACADE(Container, PRO_MAKE_DISPATCH_PACK(ForEach<T>, GetSize, Append<T>));

template <class C, class T>
pro::proxy<Container<T>> AppendImpl(C& container, T&& v) {
  container.push_back(std::move(v));
  return &container;
}
template <class T>
PRO_DEF_FREE_DISPATCH(Append, AppendImpl, pro::proxy<Container<T>>(T));

PRO_DEF_MEMBER_DISPATCH_WITH_DEFAULT(WeakAt, at, NotImplemented<std::string>, std::string(int));
PRO_DEF_FACADE(ResourceDictionary, WeakAt);

template <class F, class T>
pro::proxy<F> LockImpl(const std::weak_ptr<T>& p) {
  auto result = p.lock();
  if (static_cast<bool>(result)) {
    return result;
  }
  return nullptr;
}
template <class F>
PRO_DEF_FREE_DISPATCH(Lock, LockImpl<F>, pro::proxy<F>());
template <class F>
PRO_DEF_FACADE(Weak, Lock<F>, pro::copyable_ptr_constraints);
template <class F, class T>
auto GetWeakImpl(const std::shared_ptr<T>& p) { return pro::make_proxy<Weak<F>, std::weak_ptr<T>>(p); }
template <class F>
PRO_DEF_FREE_DISPATCH_WITH_DEFAULT(GetWeak, GetWeakImpl<F>, std::nullptr_t, pro::proxy<Weak<F>>());

PRO_DEF_FACADE(SharedStringable, PRO_MAKE_DISPATCH_PACK(utils::spec::ToString, GetWeak<SharedStringable>), pro::copyable_ptr_constraints);

}  // namespace spec

template <class F, class D, bool NE, class... Args>
concept InvocableWithDispatch =
    requires(const pro::proxy<F> p, Args... args) {
      { p.template invoke<D>(std::forward<Args>(args)...) };
      typename std::enable_if_t<NE == noexcept(p.template invoke<D>(std::forward<Args>(args)...))>;
    };
template <class F, bool NE, class... Args>
concept InvocableWithoutDispatch =
  requires(const pro::proxy<F> p, Args... args) {
    { p(std::forward<Args>(args)...) };
    typename std::enable_if_t<NE == noexcept(p(std::forward<Args>(args)...))>;
};

// Static assertions for a facade of a single dispatch
static_assert(InvocableWithDispatch<spec::Callable<int(double)>, spec::Call<int(double)>, false, double>);
static_assert(!InvocableWithDispatch<spec::Callable<int(double)>, spec::Call<int(double)>, false, std::nullptr_t>);  // Wrong arguments
static_assert(!InvocableWithoutDispatch<spec::Callable<int(double)>, false, std::nullptr_t>);  // Wrong arguments
static_assert(!InvocableWithDispatch<spec::Callable<int(double)>, int(double), false, double>);  // Wrong dispatch
static_assert(InvocableWithoutDispatch<spec::Callable<int(double)>, false, float>);  // Invoking without specifying a dispatch
static_assert(InvocableWithoutDispatch<spec::Callable<int(double), void(int) noexcept>, true, int>);  // Invoking noexcept overloads
static_assert(InvocableWithoutDispatch<spec::Callable<int(double), void(int) noexcept>, false, double>);  // Invoking overloads that may throw

// Static assertions for a facade of multiple dispatches
static_assert(InvocableWithDispatch<spec::Iterable<int>, spec::GetSize, true>);
static_assert(!InvocableWithDispatch<spec::Iterable<int>, spec::ForEach<int>, false, pro::proxy<spec::Callable<void(double&)>>>);  // Wrong arguments
static_assert(!InvocableWithDispatch<spec::Iterable<int>, spec::Append<int>, false>);  // Wrong dispatch
static_assert(!InvocableWithoutDispatch<spec::Iterable<int>, false>);  // Invoking without specifying a dispatch

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
  pro::proxy<spec::Callable<int(std::string, std::vector<int>)>> p = &f;
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
  pro::proxy<spec::Callable<void()>> p = &f;
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
  pro::proxy<spec::Iterable<int>> p = &l;
  ASSERT_EQ(p.invoke<spec::GetSize>(), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  p.invoke<spec::ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 6);
}

TEST(ProxyInvocationTests, TestMultipleDispatches_Duplicated) {
  using SomeCombination = std::tuple<spec::ForEach<int>, std::tuple<spec::GetSize, spec::ForEach<int>>>;
  PRO_DEF_FACADE(DuplicatedIterable, PRO_MAKE_DISPATCH_PACK(spec::ForEach<int>, SomeCombination, spec::ForEach<int>, spec::GetSize, spec::GetSize));
  static_assert(sizeof(pro::details::facade_traits<DuplicatedIterable>::meta) ==
      sizeof(pro::details::facade_traits<spec::Iterable<int>>::meta));
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<DuplicatedIterable> p = &l;
  ASSERT_EQ(p.invoke<spec::GetSize>(), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  p.invoke<spec::ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 6);
}

TEST(ProxyInvocationTests, TestRecursiveDefinition) {
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<spec::Container<int>> p = &l;
  ASSERT_EQ(p.invoke<spec::GetSize>(), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  p.invoke<spec::ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 6);
  p.invoke<spec::Append<int>>(4).invoke<spec::Append<int>>(5).invoke<spec::Append<int>>(6);
  ASSERT_EQ(p.invoke<spec::GetSize>(), 6);
  sum = 0;
  p.invoke<spec::ForEach<int>>(&accumulate_sum);
  ASSERT_EQ(sum, 21);
}

TEST(ProxyInvocationTests, TestAccessor) {
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<spec::Container<int>> p = &l;
  ASSERT_EQ(p.GetSize(), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  p.ForEach(&accumulate_sum);
  ASSERT_EQ(sum, 6);
  p.Append(4).Append(5).Append(6);
  ASSERT_EQ(p.GetSize(), 6);
  sum = 0;
  p.ForEach(&accumulate_sum);
  ASSERT_EQ(sum, 21);
}

TEST(ProxyInvocationTests, TestOverloadResolution) {
  PRO_DEF_FACADE(OverloadedCallable, spec::Call<void(int), void(double), void(const char*), void(char*), void(std::string, int)>);
  std::vector<std::type_index> side_effect;
  auto p = pro::make_proxy<OverloadedCallable>([&](auto&&... args)
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

TEST(ProxyInvocationTests, TestNoexcept) {
  std::vector<std::type_index> side_effect;
  auto p = pro::make_proxy<spec::Callable<void(int) noexcept, void(double)>>([&](auto&&... args) noexcept
    { side_effect = GetTypeIndices<std::decay_t<decltype(args)>...>(); });
  static_assert(noexcept(p(123)));
  p(123);
  ASSERT_EQ(side_effect, GetTypeIndices<int>());
  static_assert(!noexcept(p(1.23)));
  p(1.23);
  ASSERT_EQ(side_effect, GetTypeIndices<double>());
  ASSERT_FALSE((std::is_invocable_v<decltype(p), char*>));
}

TEST(ProxyInvocationTests, TestFunctionPointer) {
  struct TestFacade : spec::Callable<std::vector<std::type_index>()> {};
  pro::proxy<TestFacade> p{ &GetTypeIndices<int, double> };
  auto ret = p();
  ASSERT_EQ(ret, (GetTypeIndices<int, double>()));
}

TEST(ProxyInvocationTests, TestMemberDispatchDefault) {
  std::vector<std::string> container1{ "hello", "world", "!"};
  std::list<std::string> container2{ "hello", "world" };
  pro::proxy<spec::ResourceDictionary> p = &container1;
  ASSERT_EQ(p(0), "hello");
  p = &container2;
  {
    bool exception_thrown = false;
    try {
      p(0);
    } catch (const std::runtime_error& e) {
      exception_thrown = true;
      ASSERT_EQ(static_cast<std::string>(e.what()), "Not implemented!");
    }
    ASSERT_TRUE(exception_thrown);
  }
}

TEST(ProxyInvocationTests, TestFreeDispatchDefault) {
  {
    int side_effect = 0;
    auto p = pro::make_proxy<spec::WeakCallable<void()>>([&] { side_effect = 1; });
    p();
    ASSERT_EQ(side_effect, 1);
  }
  {
    bool exception_thrown = false;
    auto p = pro::make_proxy<spec::WeakCallable<void()>>(123);
    try {
      p();
    } catch (const std::runtime_error& e) {
      exception_thrown = true;
      ASSERT_EQ(static_cast<std::string>(e.what()), "Not implemented!");
    }
    ASSERT_TRUE(exception_thrown);
  }
}

TEST(ProxyInvocationTests, TestObserverDispatch) {
  int test_val = 123;
  pro::proxy<spec::SharedStringable> p{std::make_shared<int>(test_val)};
  auto weak = p.invoke<spec::GetWeak<spec::SharedStringable>>();
  ASSERT_TRUE(weak.has_value());
  {
    auto locked = weak();
    ASSERT_TRUE(locked.has_value());
    ASSERT_EQ(locked.invoke<utils::spec::ToString>(), "123");
  }
  p = &test_val;  // The underlying std::shared_ptr will be destroyed
  ASSERT_TRUE(weak.has_value());
  ASSERT_FALSE(weak().has_value());
  ASSERT_FALSE(p.invoke<spec::GetWeak<spec::SharedStringable>>().has_value());
  ASSERT_EQ(p.invoke<utils::spec::ToString>(), "123");
}
