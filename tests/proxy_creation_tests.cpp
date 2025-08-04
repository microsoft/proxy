// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "utils.h"
#include <gtest/gtest.h>
#include <memory_resource>
#include <proxy/proxy.h>

namespace proxy_creation_tests_details {

enum LifetimeModelType {
  kNone,
  kInplace,
  kAllocated,
  kCompact,
  kSharedCompact,
  kStrongCompact
};

struct LifetimeModelReflector {
  template <class T>
  constexpr explicit LifetimeModelReflector(
      std::in_place_type_t<pro::details::inplace_ptr<T>>)
      : Type(LifetimeModelType::kInplace) {}
  template <class T, class Alloc>
  constexpr explicit LifetimeModelReflector(
      std::in_place_type_t<pro::details::allocated_ptr<T, Alloc>>)
      : Type(LifetimeModelType::kAllocated) {}
  template <class T, class Alloc>
  constexpr explicit LifetimeModelReflector(
      std::in_place_type_t<pro::details::compact_ptr<T, Alloc>>)
      : Type(LifetimeModelType::kCompact) {
    static_assert(sizeof(pro::details::compact_ptr<T, Alloc>) == sizeof(void*));
  }
  template <class T, class Alloc>
  constexpr explicit LifetimeModelReflector(
      std::in_place_type_t<pro::details::shared_compact_ptr<T, Alloc>>)
      : Type(LifetimeModelType::kSharedCompact) {
    static_assert(sizeof(pro::details::shared_compact_ptr<T, Alloc>) ==
                  sizeof(void*));
  }
  template <class T, class Alloc>
  constexpr explicit LifetimeModelReflector(
      std::in_place_type_t<pro::details::strong_compact_ptr<T, Alloc>>)
      : Type(LifetimeModelType::kStrongCompact) {
    static_assert(sizeof(pro::details::strong_compact_ptr<T, Alloc>) ==
                  sizeof(void*));
  }
  template <class T>
  constexpr explicit LifetimeModelReflector(std::in_place_type_t<T>)
      : Type(LifetimeModelType::kNone) {}

  template <class Self, class R>
  struct accessor {
    LifetimeModelType GetLifetimeType() const noexcept {
      const LifetimeModelReflector& refl =
          pro::proxy_reflect<R>(static_cast<const Self&>(*this));
      return refl.Type;
    }
  };

  LifetimeModelType Type;
};

struct TestLargeStringable
    : pro::facade_builder                                        //
      ::add_convention<utils::spec::FreeToString, std::string()> //
      ::support_relocation<pro::constraint_level::nontrivial>    //
      ::support_copy<pro::constraint_level::nontrivial>          //
      ::add_direct_reflection<LifetimeModelReflector>            //
      ::build {};

struct TestSmallStringable : pro::facade_builder               //
                             ::add_facade<TestLargeStringable> //
                             ::restrict_layout<sizeof(void*)>  //
                             ::build {};

PRO_DEF_MEM_DISPATCH(MemFn0, MemFn0);
struct TestMemFn0 : pro::facade_builder                          //
                    ::add_convention<MemFn0, void(int) noexcept> //
                    ::build {};
struct TestMemFn0_Normal {
  void MemFn0(int) noexcept {}
};
static_assert(pro::proxiable<TestMemFn0_Normal*, TestMemFn0>);
struct TestMemFn0_Unsupproted {
  void MemFn1(int) noexcept {}
};
static_assert(!pro::proxiable<TestMemFn0_Unsupproted*, TestMemFn0>);
struct TestMemFn0_MissingNoexcept {
  void MemFn0(int) {}
};
static_assert(!pro::proxiable<TestMemFn0_MissingNoexcept*, TestMemFn0>);
struct TestMemFn0_ArgumentConvertible {
  void MemFn0(std::int64_t&&) noexcept {}
};
static_assert(pro::proxiable<TestMemFn0_ArgumentConvertible*, TestMemFn0>);
struct TestMemFn0_ArgumentNotMatch {
  void MemFn0(int&) noexcept {}
};
static_assert(!pro::proxiable<TestMemFn0_ArgumentNotMatch*, TestMemFn0>);
struct TestMemFn0_ReturnTypeNotMatch {
  std::string MemFn0(int) noexcept { return {}; }
};
static_assert(pro::proxiable<TestMemFn0_ReturnTypeNotMatch*, TestMemFn0>);

PRO_DEF_MEM_DISPATCH(MemFn1, MemFn1);
struct TestMemFn1 : pro::facade_builder                   //
                    ::add_convention<MemFn1, int(double)> //
                    ::build {};
struct TestMemFn1_Normal {
  int MemFn1(double) { return 0; }
};
static_assert(pro::proxiable<TestMemFn1_Normal*, TestMemFn1>);
struct TestMemFn1_HasNoexcept {
  int MemFn1(double) noexcept { return 0; }
};
static_assert(pro::proxiable<TestMemFn1_HasNoexcept*, TestMemFn1>);
struct TestMemFn1_ReturnTypeConvertible {
  std::int8_t MemFn1(double) { return 0; }
};
static_assert(pro::proxiable<TestMemFn1_ReturnTypeConvertible*, TestMemFn1>);
struct TestMemFn1_ReturnTypeNotConvertible {
  std::string MemFn1(double) { return {}; }
};
static_assert(
    !pro::proxiable<TestMemFn1_ReturnTypeNotConvertible*, TestMemFn1>);
struct TestMemFn1_ReturnTypeNotExist {
  void MemFn1(double) {}
};
static_assert(!pro::proxiable<TestMemFn1_ReturnTypeNotExist*, TestMemFn1>);

static_assert(pro::inplace_proxiable_target<utils::LifetimeTracker::Session,
                                            TestLargeStringable>);
static_assert(!pro::inplace_proxiable_target<utils::LifetimeTracker::Session,
                                             TestSmallStringable>);
static_assert(!noexcept(pro::make_proxy_inplace<
                        TestLargeStringable, utils::LifetimeTracker::Session>(
    std::declval<utils::LifetimeTracker*>())));
static_assert(noexcept(pro::make_proxy_inplace<TestLargeStringable, int>(123)));

static_assert(pro::proxiable_target<utils::LifetimeTracker::Session,
                                    TestLargeStringable>);
static_assert(pro::proxiable_target<utils::LifetimeTracker::Session,
                                    TestSmallStringable>);

template <class T>
void SfinaeUnsafeIncrementImpl(T&& value) {
  ++value;
}

PRO_DEF_FREE_DISPATCH(FreeSfinaeUnsafeIncrement, SfinaeUnsafeIncrementImpl,
                      Increment);

struct SfinaeUnsafeFacade
    : pro::facade_builder                                 //
      ::add_skill<pro::skills::rtti>                      //
      ::add_convention<FreeSfinaeUnsafeIncrement, void()> //
      ::build {};

struct TestSharedStringable
    : pro::facade_builder                                        //
      ::support_relocation<pro::constraint_level::nothrow>       //
      ::support_copy<pro::constraint_level::nothrow>             //
      ::add_convention<utils::spec::FreeToString, std::string()> //
      ::add_direct_reflection<LifetimeModelReflector>            //
      ::build {};

struct TestWeakSharedStringable : pro::facade_builder                      //
                                  ::add_facade<TestSharedStringable, true> //
                                  ::add_skill<pro::skills::as_weak>        //
                                  ::build {};

static_assert(pro::proxiable<int*, TestSharedStringable>);
static_assert(!pro::proxiable<int*, TestWeakSharedStringable>);

} // namespace proxy_creation_tests_details

namespace details = proxy_creation_tests_details;

TEST(ProxyCreationTests, TestMakeProxyInplace_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{&tracker};
  expected_ops.emplace_back(1,
                            utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::make_proxy_inplace<details::TestLargeStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kInplace);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyInplace_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy_inplace<details::TestLargeStringable,
                                     utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kInplace);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyInplace_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy_inplace<details::TestLargeStringable,
                                     utils::LifetimeTracker::Session>({1, 2, 3},
                                                                      &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kInplace);
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyInplace_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 =
        pro::make_proxy_inplace<details::TestLargeStringable,
                                utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_EQ(p1.GetLifetimeType(), details::LifetimeModelType::kInplace);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kInplace);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
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
    auto p1 =
        pro::make_proxy_inplace<details::TestLargeStringable,
                                utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kInplace);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_DirectAllocator_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{&tracker};
  expected_ops.emplace_back(1,
                            utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::allocate_proxy<details::TestSmallStringable>(
        std::allocator<void>{}, session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_DirectAllocator_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::allocate_proxy<details::TestSmallStringable,
                                 utils::LifetimeTracker::Session>(
        std::allocator<void>{}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests,
     TestAllocateProxy_DirectAllocator_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::allocate_proxy<details::TestSmallStringable,
                                 utils::LifetimeTracker::Session>(
        std::allocator<void>{}, {1, 2, 3}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_DirectAllocator_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::allocate_proxy<details::TestSmallStringable,
                                  utils::LifetimeTracker::Session>(
        std::allocator<void>{}, &tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_EQ(p1.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
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
    auto p1 = pro::allocate_proxy<details::TestSmallStringable,
                                  utils::LifetimeTracker::Session>(
        std::allocator<void>{}, &tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxy_IndirectAllocator_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{&tracker};
  expected_ops.emplace_back(1,
                            utils::LifetimeOperationType::kValueConstruction);
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy<details::TestSmallStringable>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kCompact);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
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
    auto p = pro::allocate_proxy<details::TestSmallStringable,
                                 utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kCompact);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests,
     TestAllocateProxy_IndirectAllocator_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy<details::TestSmallStringable,
                                 utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, {1, 2, 3}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kCompact);
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
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
    auto p1 = pro::allocate_proxy<details::TestSmallStringable,
                                  utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, &tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_EQ(p1.GetLifetimeType(), details::LifetimeModelType::kCompact);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kCompact);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
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
    auto p1 = pro::allocate_proxy<details::TestSmallStringable,
                                  utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, &tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kCompact);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithSBO_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{&tracker};
  expected_ops.emplace_back(1,
                            utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::make_proxy<details::TestLargeStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kInplace);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithSBO_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy<details::TestLargeStringable,
                             utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kInplace);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithSBO_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p =
        pro::make_proxy<details::TestLargeStringable,
                        utils::LifetimeTracker::Session>({1, 2, 3}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kInplace);
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithSBO_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy<details::TestLargeStringable,
                              utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_EQ(p1.GetLifetimeType(), details::LifetimeModelType::kInplace);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kInplace);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
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
    auto p1 = pro::make_proxy<details::TestLargeStringable,
                              utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kInplace);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithoutSBO_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{&tracker};
  expected_ops.emplace_back(1,
                            utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::make_proxy<details::TestSmallStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithoutSBO_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy<details::TestSmallStringable,
                             utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithoutSBO_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p =
        pro::make_proxy<details::TestSmallStringable,
                        utils::LifetimeTracker::Session>({1, 2, 3}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_WithoutSBO_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy<details::TestSmallStringable,
                              utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_EQ(p1.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
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
    auto p1 = pro::make_proxy<details::TestSmallStringable,
                              utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kAllocated);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxy_SfinaeUnsafe) {
  pro::proxy<details::SfinaeUnsafeFacade> p =
      pro::make_proxy<details::SfinaeUnsafeFacade, int>();
  Increment(*p);
  ASSERT_EQ(proxy_cast<int>(*p), 1);
}

TEST(ProxyCreationTests, TestAllocateProxyShared_SharedCompact_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{&tracker};
  expected_ops.emplace_back(1,
                            utils::LifetimeOperationType::kValueConstruction);
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy_shared<details::TestSharedStringable>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxyShared_SharedCompact_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy_shared<details::TestSharedStringable,
                                        utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests,
     TestAllocateProxyShared_SharedCompact_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy_shared<details::TestSharedStringable,
                                        utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, {1, 2, 3}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxyShared_SharedCompact_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p1 = pro::allocate_proxy_shared<details::TestSharedStringable,
                                         utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, &tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_EQ(p1.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxyShared_SharedCompact_Lifetime_Move) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p1 = pro::allocate_proxy_shared<details::TestSharedStringable,
                                         utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, &tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxyShared_StrongCompact_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{&tracker};
  expected_ops.emplace_back(1,
                            utils::LifetimeOperationType::kValueConstruction);
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy_shared<details::TestWeakSharedStringable>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxyShared_StrongCompact_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy_shared<details::TestWeakSharedStringable,
                                        utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests,
     TestAllocateProxyShared_StrongCompact_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p = pro::allocate_proxy_shared<details::TestWeakSharedStringable,
                                        utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, {1, 2, 3}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxyShared_StrongCompact_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p1 = pro::allocate_proxy_shared<details::TestWeakSharedStringable,
                                         utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, &tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_EQ(p1.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestAllocateProxyShared_StrongCompact_Lifetime_Move) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p1 = pro::allocate_proxy_shared<details::TestWeakSharedStringable,
                                         utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, &tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests,
     TestAllocateProxyShared_StrongCompact_Lifetime_WeakAccess) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    std::pmr::unsynchronized_pool_resource memory_pool;
    auto p1 = pro::allocate_proxy_shared<details::TestWeakSharedStringable,
                                         utils::LifetimeTracker::Session>(
        std::pmr::polymorphic_allocator<>{&memory_pool}, &tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::weak_proxy<details::TestWeakSharedStringable> p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    auto p3 = p2.lock();
    ASSERT_TRUE(p3.has_value());
    ASSERT_EQ(ToString(*p3), "Session 1");
    ASSERT_EQ(p3.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    p3.reset();
    p1.reset();
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    auto p4 = p2.lock();
    ASSERT_FALSE(p4.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyShared_SharedCompact_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{&tracker};
  expected_ops.emplace_back(1,
                            utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::make_proxy_shared<details::TestSharedStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyShared_SharedCompact_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy_shared<details::TestSharedStringable,
                                    utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests,
     TestMakeProxyShared_SharedCompact_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy_shared<details::TestSharedStringable,
                                    utils::LifetimeTracker::Session>({1, 2, 3},
                                                                     &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyShared_SharedCompact_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy_shared<details::TestSharedStringable,
                                     utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_EQ(p1.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyShared_SharedCompact_Lifetime_Move) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy_shared<details::TestSharedStringable,
                                     utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kSharedCompact);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyShared_StrongCompact_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{&tracker};
  expected_ops.emplace_back(1,
                            utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::make_proxy_shared<details::TestWeakSharedStringable>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyShared_StrongCompact_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy_shared<details::TestWeakSharedStringable,
                                    utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests,
     TestMakeProxyShared_StrongCompact_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p = pro::make_proxy_shared<details::TestWeakSharedStringable,
                                    utils::LifetimeTracker::Session>({1, 2, 3},
                                                                     &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(p.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyShared_StrongCompact_Lifetime_Copy) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy_shared<details::TestWeakSharedStringable,
                                     utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_EQ(p1.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyShared_StrongCompact_Lifetime_Move) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy_shared<details::TestWeakSharedStringable,
                                     utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_EQ(p2.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests,
     TestMakeProxyShared_StrongCompact_Lifetime_WeakAccess) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy_shared<details::TestWeakSharedStringable,
                                     utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::weak_proxy<details::TestWeakSharedStringable> p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    auto p3 = p2.lock();
    ASSERT_TRUE(p3.has_value());
    ASSERT_EQ(ToString(*p3), "Session 1");
    ASSERT_EQ(p3.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    p3.reset();
    p1.reset();
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    auto p4 = p2.lock();
    ASSERT_FALSE(p4.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests,
     TestMakeProxyShared_StrongCompact_Lifetime_WeakConversion) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    auto p1 = pro::make_proxy_shared<details::TestWeakSharedStringable,
                                     utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::weak_proxy<details::TestWeakSharedStringable> p2 = p1;
    pro::weak_proxy<details::TestSharedStringable> p3 = std::move(p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_FALSE(p2.has_value());
    ASSERT_TRUE(p3.has_value());
    auto p4 = p3.lock();
    ASSERT_TRUE(p4.has_value());
    ASSERT_EQ(ToString(*p4), "Session 1");
    ASSERT_EQ(p4.GetLifetimeType(), details::LifetimeModelType::kStrongCompact);
    p4.reset();
    p1.reset();
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    auto p5 = p3.lock();
    ASSERT_FALSE(p5.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestStdWeakPtrCompatibility) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestWeakSharedStringable> p1 =
        std::make_shared<utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::weak_proxy<details::TestWeakSharedStringable> p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    auto p3 = p2.lock();
    ASSERT_TRUE(p3.has_value());
    ASSERT_EQ(ToString(*p3), "Session 1");
    p3.reset();
    p1.reset();
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    auto p4 = p2.lock();
    ASSERT_FALSE(p4.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyCreationTests, TestMakeProxyView) {
  struct TestFacade
      : pro::facade_builder //
        ::add_convention<pro::operator_dispatch<"()">, int() &, int() const&,
                         int() && noexcept, int() const&&> //
        ::build {};

  struct {
    int operator()() & noexcept { return 0; }
    int operator()() const& noexcept { return 1; }
    int operator()() && noexcept { return 2; }
    int operator()() const&& noexcept { return 3; }
  } test_callable;

  pro::proxy_view<TestFacade> p =
      pro::make_proxy_view<TestFacade>(test_callable);
  static_assert(!noexcept((*p)()));
  static_assert(noexcept((*std::move(p))()));
  ASSERT_EQ((*p)(), 0);
  ASSERT_EQ((*std::as_const(p))(), 1);
  ASSERT_EQ((*std::move(p))(), 2);
  p = pro::make_proxy_view<TestFacade>(test_callable);
  ASSERT_EQ((*std::move(std::as_const(p)))(), 3);
}
