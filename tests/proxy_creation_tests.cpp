// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <memory_resource>
#include "proxy.h"
#include "utils.h"

namespace {

struct SboObserver {
 public:
  template <class T>
  constexpr explicit SboObserver(std::in_place_type_t<pro::details::inplace_ptr<T>>)
      : SboEnabled(true), AllocatorAllocatesForItself(false) {}
  template <class T, class Alloc>
  constexpr explicit SboObserver(std::in_place_type_t<pro::details::allocated_ptr<T, Alloc>>)
      : SboEnabled(false), AllocatorAllocatesForItself(false) {}
  template <class T, class Alloc>
  constexpr explicit SboObserver(std::in_place_type_t<pro::details::compact_ptr<T, Alloc>>)
      : SboEnabled(false), AllocatorAllocatesForItself(true) {}

  bool SboEnabled;
  bool AllocatorAllocatesForItself;
};

namespace spec {

PRO_DEF_FACADE(TestSmallStringable, utils::spec::ToString, pro::proxiable_ptr_constraints{
    .max_size = sizeof(void*),
    .max_align = alignof(void*),
    .copyability = pro::constraint_level::nontrivial,
    .relocatability = pro::constraint_level::nothrow,
    .destructibility = pro::constraint_level::nothrow,
  }, SboObserver);
PRO_DEF_FACADE(TestLargeStringable, utils::spec::ToString, pro::copyable_ptr_constraints, SboObserver);

}  // namespace spec

PRO_DEF_MEMBER_DISPATCH(MemFn0, void(int) noexcept);
PRO_DEF_FACADE(TestMemFn0, MemFn0);
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

PRO_DEF_MEMBER_DISPATCH(MemFn1, int(double));
PRO_DEF_FACADE(TestMemFn1, MemFn1);
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

static_assert(pro::inplace_proxiable_target<utils::LifetimeTracker::Session, spec::TestLargeStringable>);
static_assert(!pro::inplace_proxiable_target<utils::LifetimeTracker::Session, spec::TestSmallStringable>);
static_assert(!noexcept(pro::make_proxy_inplace<spec::TestLargeStringable, utils::LifetimeTracker::Session>(std::declval<utils::LifetimeTracker*>())));
static_assert(noexcept(pro::make_proxy_inplace<spec::TestLargeStringable, int>(123)));

}  // namespace

TEST(ProxyCreationTests, TestMakeProxyInplace_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{ &tracker };
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::make_proxy_inplace<spec::TestLargeStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 2");
    ASSERT_TRUE(p.reflect().SboEnabled);
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
    auto p = pro::make_proxy_inplace<spec::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_TRUE(p.reflect().SboEnabled);
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
    auto p = pro::make_proxy_inplace<spec::TestLargeStringable, utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_TRUE(p.reflect().SboEnabled);
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
    auto p1 = pro::make_proxy_inplace<spec::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 1");
    ASSERT_TRUE(p1.reflect().SboEnabled);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    ASSERT_TRUE(p2.reflect().SboEnabled);
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
    auto p1 = pro::make_proxy_inplace<spec::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    ASSERT_TRUE(p2.reflect().SboEnabled);
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
    auto p = pro::allocate_proxy<spec::TestSmallStringable>(std::allocator<void>{}, session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 2");
    ASSERT_FALSE(p.reflect().SboEnabled);
    ASSERT_FALSE(p.reflect().AllocatorAllocatesForItself);
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
    auto p = pro::allocate_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(std::allocator<void>{}, & tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_FALSE(p.reflect().SboEnabled);
    ASSERT_FALSE(p.reflect().AllocatorAllocatesForItself);
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
    auto p = pro::allocate_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(std::allocator<void>{}, { 1, 2, 3 }, & tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_FALSE(p.reflect().SboEnabled);
    ASSERT_FALSE(p.reflect().AllocatorAllocatesForItself);
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
    auto p1 = pro::allocate_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(std::allocator<void>{}, & tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 1");
    ASSERT_FALSE(p1.reflect().SboEnabled);
    ASSERT_FALSE(p1.reflect().AllocatorAllocatesForItself);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    ASSERT_FALSE(p2.reflect().SboEnabled);
    ASSERT_FALSE(p2.reflect().AllocatorAllocatesForItself);
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
    auto p1 = pro::allocate_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(std::allocator<void>{}, & tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 1");
    ASSERT_FALSE(p2.reflect().SboEnabled);
    ASSERT_FALSE(p2.reflect().AllocatorAllocatesForItself);
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
    auto p = pro::allocate_proxy<spec::TestSmallStringable>(std::pmr::polymorphic_allocator<>{&memory_pool}, session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 2");
    ASSERT_FALSE(p.reflect().SboEnabled);
    ASSERT_TRUE(p.reflect().AllocatorAllocatesForItself);
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
    auto p = pro::allocate_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(std::pmr::polymorphic_allocator<>{&memory_pool}, & tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_FALSE(p.reflect().SboEnabled);
    ASSERT_TRUE(p.reflect().AllocatorAllocatesForItself);
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
    auto p = pro::allocate_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(std::pmr::polymorphic_allocator<>{&memory_pool}, { 1, 2, 3 }, & tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_FALSE(p.reflect().SboEnabled);
    ASSERT_TRUE(p.reflect().AllocatorAllocatesForItself);
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
    auto p1 = pro::allocate_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(std::pmr::polymorphic_allocator<>{&memory_pool}, & tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 1");
    ASSERT_FALSE(p1.reflect().SboEnabled);
    ASSERT_TRUE(p1.reflect().AllocatorAllocatesForItself);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    ASSERT_FALSE(p2.reflect().SboEnabled);
    ASSERT_TRUE(p2.reflect().AllocatorAllocatesForItself);
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
    auto p1 = pro::allocate_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(std::pmr::polymorphic_allocator<>{&memory_pool}, & tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 1");
    ASSERT_FALSE(p2.reflect().SboEnabled);
    ASSERT_TRUE(p2.reflect().AllocatorAllocatesForItself);
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
    auto p = pro::make_proxy<spec::TestLargeStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 2");
    ASSERT_TRUE(p.reflect().SboEnabled);
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
    auto p = pro::make_proxy<spec::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_TRUE(p.reflect().SboEnabled);
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
    auto p = pro::make_proxy<spec::TestLargeStringable, utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_TRUE(p.reflect().SboEnabled);
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
    auto p1 = pro::make_proxy<spec::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 1");
    ASSERT_TRUE(p1.reflect().SboEnabled);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    ASSERT_TRUE(p2.reflect().SboEnabled);
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
    auto p1 = pro::make_proxy<spec::TestLargeStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    ASSERT_TRUE(p2.reflect().SboEnabled);
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
    auto p = pro::make_proxy<spec::TestSmallStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 2");
    ASSERT_FALSE(p.reflect().SboEnabled);
    ASSERT_FALSE(p.reflect().AllocatorAllocatesForItself);
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
    auto p = pro::make_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_FALSE(p.reflect().SboEnabled);
    ASSERT_FALSE(p.reflect().AllocatorAllocatesForItself);
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
    auto p = pro::make_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_FALSE(p.reflect().SboEnabled);
    ASSERT_FALSE(p.reflect().AllocatorAllocatesForItself);
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
    auto p1 = pro::make_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 1");
    ASSERT_FALSE(p1.reflect().SboEnabled);
    ASSERT_FALSE(p1.reflect().AllocatorAllocatesForItself);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    ASSERT_FALSE(p2.reflect().SboEnabled);
    ASSERT_FALSE(p2.reflect().AllocatorAllocatesForItself);
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
    auto p1 = pro::make_proxy<spec::TestSmallStringable, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 1");
    ASSERT_FALSE(p2.reflect().SboEnabled);
    ASSERT_FALSE(p2.reflect().AllocatorAllocatesForItself);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}
