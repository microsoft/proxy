// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <ranges>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(push)
#pragma warning(disable: 4702)  // False alarm from MSVC: warning C4702: unreachable code
#endif  // defined(_MSC_VER) && !defined(__clang__)
#include "proxy.h"
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(pop)
#endif  // defined(_MSC_VER) && !defined(__clang__)
#include "utils.h"

namespace proxy_invocation_tests_details {

template <class... Os>
struct MovableCallable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, Os...>
    ::build {};

template <class... Os>
struct Callable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_facade<MovableCallable<Os...>>
    ::build {};

struct Wildcard {
  template <class T>
  operator T() const noexcept { std::terminate(); }
};

Wildcard NotImplemented(auto&&...) { throw std::runtime_error{ "Not implemented!" }; }

PRO_DEF_WEAK_DISPATCH(WeakOpCall, pro::operator_dispatch<"()">, NotImplemented);
template <class... Os>
struct WeakCallable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_convention<WeakOpCall, Os...>
    ::build {};

PRO_DEF_FREE_DISPATCH(FreeSize, std::ranges::size, Size);
PRO_DEF_FREE_DISPATCH(FreeForEach, std::ranges::for_each, ForEach);

template <class T>
struct Iterable : pro::facade_builder
    ::add_convention<FreeForEach, void(std::function<void(T&)>)>
    ::template add_convention<FreeSize, std::size_t() noexcept>
    ::build {};

template <class T>
struct Container;

template <class C, class T>
pro::proxy<Container<T>> AppendImpl(C& container, T&& v) {
  container.push_back(std::move(v));
  return &container;
}

PRO_DEF_FREE_DISPATCH(FreeAppend, AppendImpl, Append);

template <class T>
struct Container : pro::facade_builder
    ::add_facade<Iterable<T>>
    ::template add_convention<FreeAppend, pro::proxy<Container<T>>(T) const&>
    ::build {};

PRO_DEF_MEM_DISPATCH(MemAt, at, at);
PRO_DEF_WEAK_DISPATCH(MemAtWeak, MemAt, NotImplemented);

struct ResourceDictionary : pro::facade_builder
    ::add_convention<MemAtWeak, std::string(int)>
    ::build {};

template <class F, class T>
pro::proxy<F> LockImpl(const std::weak_ptr<T>& p) {
  auto result = p.lock();
  if (static_cast<bool>(result)) {
    return result;
  }
  return nullptr;
}
template <class F>
PRO_DEF_FREE_DISPATCH(FreeLock, LockImpl<F>, Lock);

template <class F>
struct Weak : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_convention<FreeLock<F>, pro::proxy<F>()>
    ::build {};

template <class F, class T>
auto GetWeakImpl(const std::shared_ptr<T>& p) { return pro::make_proxy<Weak<F>, std::weak_ptr<T>>(p); }

template <class F>
PRO_DEF_FREE_DISPATCH(FreeGetWeakImpl, GetWeakImpl<F>, GetWeak);
template <class F>
PRO_DEF_WEAK_DISPATCH(FreeGetWeak, FreeGetWeakImpl<F>, std::nullptr_t);

struct SharedStringable : pro::facade_builder
    ::add_facade<utils::spec::Stringable>
    ::add_direct_convention<FreeGetWeak<SharedStringable>, pro::proxy<Weak<SharedStringable>>() const&>
    ::build {};

template <class F, bool NE, class... Args>
concept CallableFacade =
  requires(pro::proxy<F> p, Args... args) {
    { (*p)(std::forward<Args>(args)...) };
    typename std::enable_if_t<NE == noexcept((*p)(std::forward<Args>(args)...))>;
};

// Static assertions for facade Callable
static_assert(!CallableFacade<Callable<int(double)>, false, std::nullptr_t>);  // Wrong arguments
static_assert(CallableFacade<Callable<int(double)>, false, float>);  // Invoking without specifying a dispatch
static_assert(CallableFacade<Callable<int(double), void(int) noexcept>, true, int>);  // Invoking noexcept overloads
static_assert(CallableFacade<Callable<int(double), void(int) noexcept>, false, double>);  // Invoking overloads that may throw

template <class... Args>
std::vector<std::type_index> GetTypeIndices()
    { return {std::type_index{typeid(Args)}...}; }

}  // namespace proxy_invocation_tests_details

namespace details = proxy_invocation_tests_details;

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
  pro::proxy<details::Callable<int(std::string, std::vector<int>)>> p = &f;
  int result = (*p)(arg1, std::move(arg2));
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
  pro::proxy<details::Callable<void()>> p = &f;
  try {
    (*p)();
  } catch (const std::runtime_error& e) {
    exception_thrown = true;
    ASSERT_STREQ(e.what(), expected_error_message);
  }
  ASSERT_TRUE(exception_thrown);
  ASSERT_TRUE(p.has_value());
}

TEST(ProxyInvocationTests, TestMultipleDispatches_Unique) {
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<details::Iterable<int>> p = &l;
  ASSERT_EQ(Size(*p), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  ForEach(*p, accumulate_sum);
  ASSERT_EQ(sum, 6);
}

TEST(ProxyInvocationTests, TestMultipleDispatches_Duplicated) {
  struct DuplicatedIterable : pro::facade_builder
      ::add_convention<details::FreeForEach, void(std::function<void(int&)>)>
      ::add_convention<details::FreeSize, std::size_t()>
      ::add_convention<details::FreeForEach, void(std::function<void(int&)>)>
      ::build {};
  static_assert(sizeof(pro::details::facade_traits<DuplicatedIterable>::meta) ==
      sizeof(pro::details::facade_traits<details::Iterable<int>>::meta));
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<DuplicatedIterable> p = &l;
  ASSERT_EQ(Size(*p), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  ForEach(*p, accumulate_sum);
  ASSERT_EQ(sum, 6);
}

TEST(ProxyInvocationTests, TestRecursiveDefinition) {
  std::list<int> l = { 1, 2, 3 };
  pro::proxy<details::Container<int>> p = &l;
  ASSERT_EQ(Size(*p), 3);
  int sum = 0;
  auto accumulate_sum = [&](int x) { sum += x; };
  ForEach(*p, accumulate_sum);
  ASSERT_EQ(sum, 6);
  Append(*Append(*Append(*p, 4), 5), 6);
  ASSERT_EQ(Size(*p), 6);
  sum = 0;
  ForEach(*p, accumulate_sum);
  ASSERT_EQ(sum, 21);
}

TEST(ProxyInvocationTests, TestOverloadResolution) {
  struct OverloadedCallable : pro::facade_builder
      ::add_convention<pro::operator_dispatch<"()">, void(int), void(double), void(const char*), void(char*), void(std::string, int)>
      ::build {};
  std::vector<std::type_index> side_effect;
  auto p = pro::make_proxy<OverloadedCallable>([&](auto&&... args)
      { side_effect = details::GetTypeIndices<std::decay_t<decltype(args)>...>(); });
  (*p)(123);
  ASSERT_EQ(side_effect, details::GetTypeIndices<int>());
  (*p)(1.23);
  ASSERT_EQ(side_effect, details::GetTypeIndices<double>());
  char foo[2];
  (*p)(foo);
  ASSERT_EQ(side_effect, details::GetTypeIndices<char*>());
  (*p)("lalala");
  ASSERT_EQ(side_effect, details::GetTypeIndices<const char*>());
  (*p)("lalala", 0);
  ASSERT_EQ(side_effect, (details::GetTypeIndices<std::string, int>()));
  ASSERT_FALSE((std::is_invocable_v<decltype(*p), std::vector<int>>));
}

TEST(ProxyInvocationTests, TestNoexcept) {
  std::vector<std::type_index> side_effect;
  auto p = pro::make_proxy<details::Callable<void(int) noexcept, void(double)>>([&](auto&&... args) noexcept
    { side_effect = details::GetTypeIndices<std::decay_t<decltype(args)>...>(); });
  static_assert(noexcept((*p)(123)));
  (*p)(123);
  ASSERT_EQ(side_effect, details::GetTypeIndices<int>());
  static_assert(!noexcept((*p)(1.23)));
  (*p)(1.23);
  ASSERT_EQ(side_effect, details::GetTypeIndices<double>());
  ASSERT_FALSE((std::is_invocable_v<decltype(*p), char*>));
}

TEST(ProxyInvocationTests, TestFunctionPointer) {
  struct TestFacade : details::Callable<std::vector<std::type_index>()> {};
  pro::proxy<TestFacade> p{ &details::GetTypeIndices<int, double> };
  auto ret = (*p)();
  ASSERT_EQ(ret, (details::GetTypeIndices<int, double>()));
}

TEST(ProxyInvocationTests, TestMemberDispatchDefault) {
  std::vector<std::string> container1{ "hello", "world", "!"};
  std::list<std::string> container2{ "hello", "world" };
  pro::proxy<details::ResourceDictionary> p = &container1;
  ASSERT_EQ(p->at(0), "hello");
  p = &container2;
  {
    bool exception_thrown = false;
    try {
      p->at(0);
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
    auto p = pro::make_proxy<details::WeakCallable<void()>>([&] { side_effect = 1; });
    (*p)();
    ASSERT_EQ(side_effect, 1);
  }
  {
    bool exception_thrown = false;
    auto p = pro::make_proxy<details::WeakCallable<void()>>(123);
    try {
      (*p)();
    } catch (const std::runtime_error& e) {
      exception_thrown = true;
      ASSERT_EQ(static_cast<std::string>(e.what()), "Not implemented!");
    }
    ASSERT_TRUE(exception_thrown);
  }
}

TEST(ProxyInvocationTests, TestObserverDispatch) {
  int test_val = 123;
  pro::proxy<details::SharedStringable> p{std::make_shared<int>(test_val)};
  auto weak = GetWeak(p);
  ASSERT_TRUE(weak.has_value());
  {
    auto locked = Lock(*weak);
    ASSERT_TRUE(locked.has_value());
    ASSERT_EQ(ToString(*locked), "123");
  }
  p = &test_val;  // The underlying std::shared_ptr will be destroyed
  ASSERT_TRUE(weak.has_value());
  ASSERT_FALSE(Lock(*weak).has_value());
  ASSERT_FALSE(GetWeak(p).has_value());
  ASSERT_EQ(ToString(*p), "123");
}

TEST(ProxyInvocationTests, TestQualifiedConvention) {
  struct TestFacade : pro::facade_builder
      ::add_convention<pro::operator_dispatch<"()">, int()&, int() const&, int() && noexcept, int() const&&>
      ::build {};

  struct TestCallable {
    int operator()()& noexcept { return 0; }
    int operator()() const& noexcept { return 1; }
    int operator()() && noexcept { return 2; }
    int operator()() const&& noexcept { return 3; }
  };

  pro::proxy<TestFacade> p = pro::make_proxy<TestFacade, TestCallable>();
  static_assert(!noexcept((*p)()));
  static_assert(noexcept((*std::move(p))()));
  ASSERT_EQ((*p)(), 0);
  ASSERT_EQ((*std::as_const(p))(), 1);
  ASSERT_EQ((*std::move(p))(), 2);
  ASSERT_EQ((*std::move(std::as_const(p)))(), 3);
}
