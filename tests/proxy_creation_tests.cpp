// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <memory_resource>
#include "proxy.h"
#include "utils.h"

namespace proxy_creation_tests_details {

struct SboReflector {
 public:
  template <class T>
  constexpr explicit SboReflector(std::in_place_type_t<pro::details::inplace_ptr<T>>)
      : SboEnabled(true), AllocatorAllocatesForItself(false) {}
  template <class T, class Alloc>
  constexpr explicit SboReflector(std::in_place_type_t<pro::details::allocated_ptr<T, Alloc>>)
      : SboEnabled(false), AllocatorAllocatesForItself(false) {}
  template <class T, class Alloc>
  constexpr explicit SboReflector(std::in_place_type_t<pro::details::compact_ptr<T, Alloc>>)
      : SboEnabled(false), AllocatorAllocatesForItself(true) {}

  template <class F, class R>
  struct accessor {
    const SboReflector& ReflectSbo() const noexcept {
      return pro::proxy_reflect<R>(pro::access_proxy<F>(*this));
    }
  };

  bool SboEnabled;
  bool AllocatorAllocatesForItself;
};

struct TestLargeStringable : pro::facade_builder
    ::add_convention<utils::spec::FreeToString, std::string()>
    ::support_relocation<pro::constraint_level::nontrivial>
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_direct_reflection<SboReflector>
    ::build {};

struct TestSmallStringable : pro::facade_builder
    ::add_facade<TestLargeStringable>
    ::restrict_layout<sizeof(void*)>
    ::build {};

PRO_DEF_MEM_DISPATCH(MemFn0, MemFn0);
struct TestMemFn0 : pro::facade_builder
    ::add_convention<MemFn0, void(int) noexcept>
    ::build {};
struct TestMemFn0_Normal { void MemFn0(int) noexcept {} };
static_assert(pro::proxiable<TestMemFn0_Normal*, TestMemFn0>);
struct TestMemFn0_Unsupproted { void MemFn1(int) noexcept {} };
static_assert(!pro::proxiable<TestMemFn0_Unsupproted*, TestMemFn0>);
struct TestMemFn0_MissingNoexcept { void MemFn0(int) {} };
static_assert(!pro::proxiable<TestMemFn0_MissingNoexcept*, TestMemFn0>);
struct TestMemFn0_ArgumentConvertible { void MemFn0(std::int64_t&&) noexcept {} };
static_assert(pro::proxiable<TestMemFn0_ArgumentConvertible*, TestMemFn0>);
struct TestMemFn0_ArgumentNotMatch { void MemFn0(int&) noexcept {} };
static_assert(!pro::proxiable<TestMemFn0_ArgumentNotMatch*, TestMemFn0>);
struct TestMemFn0_ReturnTypeNotMatch { std::string MemFn0(int) noexcept { return {}; } };
static_assert(pro::proxiable<TestMemFn0_ReturnTypeNotMatch*, TestMemFn0>);

PRO_DEF_MEM_DISPATCH(MemFn1, MemFn1);
struct TestMemFn1 : pro::facade_builder
    ::add_convention<MemFn1, int(double)>
    ::build {};
struct TestMemFn1_Normal { int MemFn1(double) { return 0; } };
static_assert(pro::proxiable<TestMemFn1_Normal*, TestMemFn1>);
struct TestMemFn1_HasNoexcept { int MemFn1(double) noexcept { return 0; } };
static_assert(pro::proxiable<TestMemFn1_HasNoexcept*, TestMemFn1>);
struct TestMemFn1_ReturnTypeConvertible { std::int8_t MemFn1(double) { return 0; } };
static_assert(pro::proxiable<TestMemFn1_ReturnTypeConvertible*, TestMemFn1>);
struct TestMemFn1_ReturnTypeNotConvertible { std::string MemFn1(double) { return {}; } };
static_assert(!pro::proxiable<TestMemFn1_ReturnTypeNotConvertible*, TestMemFn1>);
struct TestMemFn1_ReturnTypeNotExist { void MemFn1(double) {} };
static_assert(!pro::proxiable<TestMemFn1_ReturnTypeNotExist*, TestMemFn1>);

static_assert(pro::inplace_proxiable_target<utils::LifetimeTracker::Session, TestLargeStringable>);
static_assert(!pro::inplace_proxiable_target<utils::LifetimeTracker::Session, TestSmallStringable>);
static_assert(!noexcept(pro::make_proxy_inplace<TestLargeStringable, utils::LifetimeTracker::Session>(std::declval<utils::LifetimeTracker*>())));
static_assert(noexcept(pro::make_proxy_inplace<TestLargeStringable, int>(123)));

}  // namespace proxy_creation_tests_details

namespace details = proxy_creation_tests_details;

TEST(ProxyCreationTests, TestMakeProxyInplace_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{ &tracker };
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::make_proxy_inplace<details::TestLargeStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_TRUE(p.ReflectSbo().SboEnabled);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyInplace_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy_inplace<details::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_TRUE(p.ReflectSbo().SboEnabled);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyInplace_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy_inplace<details::TestLargeStringable, utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_TRUE(p.ReflectSbo().SboEnabled);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyInplace_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy_inplace<details::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_TRUE(p1.ReflectSbo().SboEnabled);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_TRUE(p2.ReflectSbo().SboEnabled);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyInplace_Lifetime_Move) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy_inplace<details::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_TRUE(p2.ReflectSbo().SboEnabled);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_DirectAllocator_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{ &tracker };
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::allocate_proxy<details::TestSmallStringable>(std::allocator<void>{}, session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_FALSE(p.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_DirectAllocator_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::allocate_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(std::allocator<void>{}, & tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_FALSE(p.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_DirectAllocator_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::allocate_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(std::allocator<void>{}, { 1, 2, 3 }, & tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_FALSE(p.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_DirectAllocator_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::allocate_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(std::allocator<void>{}, & tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_FALSE(p1.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p1.ReflectSbo().AllocatorAllocatesForItself);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_FALSE(p2.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p2.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_DirectAllocator_Lifetime_Move) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::allocate_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(std::allocator<void>{}, & tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_FALSE(p2.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p2.ReflectSbo().AllocatorAllocatesForItself);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_IndirectAllocator_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{ &tracker };
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy<details::TestSmallStringable>(std::pmr::polymorphic_allocator<>{&memory_pool}, session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_FALSE(p.ReflectSbo().SboEnabled);
    ASSERT_TRUE(p.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_IndirectAllocator_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(std::pmr::polymorphic_allocator<>{&memory_pool}, & tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_FALSE(p.ReflectSbo().SboEnabled);
    ASSERT_TRUE(p.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_IndirectAllocator_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(std::pmr::polymorphic_allocator<>{&memory_pool}, { 1, 2, 3 }, & tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_FALSE(p.ReflectSbo().SboEnabled);
    ASSERT_TRUE(p.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_IndirectAllocator_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p1 = pro::allocate_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(std::pmr::polymorphic_allocator<>{&memory_pool}, & tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_FALSE(p1.ReflectSbo().SboEnabled);
    ASSERT_TRUE(p1.ReflectSbo().AllocatorAllocatesForItself);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_FALSE(p2.ReflectSbo().SboEnabled);
    ASSERT_TRUE(p2.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_IndirectAllocator_Lifetime_Move) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p1 = pro::allocate_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(std::pmr::polymorphic_allocator<>{&memory_pool}, & tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_FALSE(p2.ReflectSbo().SboEnabled);
    ASSERT_TRUE(p2.ReflectSbo().AllocatorAllocatesForItself);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithSBO_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{ &tracker };
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::make_proxy<details::TestLargeStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_TRUE(p.ReflectSbo().SboEnabled);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithSBO_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy<details::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_TRUE(p.ReflectSbo().SboEnabled);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithSBO_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy<details::TestLargeStringable, utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_TRUE(p.ReflectSbo().SboEnabled);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithSBO_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy<details::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_TRUE(p1.ReflectSbo().SboEnabled);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_TRUE(p2.ReflectSbo().SboEnabled);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithSBO_Lifetime_Move) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy<details::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_TRUE(p2.ReflectSbo().SboEnabled);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithoutSBO_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{ &tracker };
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::make_proxy<details::TestSmallStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_FALSE(p.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithoutSBO_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_FALSE(p.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithoutSBO_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_FALSE(p.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithoutSBO_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_FALSE(p1.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p1.ReflectSbo().AllocatorAllocatesForItself);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_FALSE(p2.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p2.ReflectSbo().AllocatorAllocatesForItself);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithoutSBO_Lifetime_Move) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy<details::TestSmallStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_FALSE(p2.ReflectSbo().SboEnabled);
    ASSERT_FALSE(p2.ReflectSbo().AllocatorAllocatesForItself);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}
