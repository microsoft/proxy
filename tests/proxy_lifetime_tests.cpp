// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <memory>
#include <memory_resource>
#include <vector>
#include "proxy.h"

// TODO: Block unit tests requireing P0848R3 for clang. Clang implementation status: https://clang.llvm.org/cxx_status.html
namespace {

template <bool kNothrowRelocatable, bool kCopyable, bool kTrivial, std::size_t kSize, std::size_t kAlignment>
struct MockPtr {
  MockPtr() = default;
  MockPtr(int) noexcept {}
  MockPtr(const MockPtr&) requires(kCopyable && !kTrivial) {}
  MockPtr(const MockPtr&) noexcept requires(kTrivial) = default;
  MockPtr(MockPtr&&) noexcept requires(kNothrowRelocatable && !kTrivial) {}
  MockPtr(MockPtr&&) noexcept requires(kTrivial) = default;
  MockPtr& operator*() noexcept { return *this; }

  alignas(kAlignment) char dummy_[kSize];
};
using MockMovablePtr = MockPtr<true, false, false, sizeof(void*) * 2, alignof(void*)>;
using MockCopyablePtr = MockPtr<true, true, false, sizeof(void*) * 2, alignof(void*)>;
using MockCopyableSmallPtr = MockPtr<true, true, false, sizeof(void*), alignof(void*)>;
using MockTrivialPtr = MockPtr<true, true, true, sizeof(void*) * 2, alignof(void*)>;

using DefaultFacade = pro::facade<>;
static_assert(DefaultFacade::minimum_copyability == pro::constraint_level::none);
static_assert(DefaultFacade::minimum_relocatability == pro::constraint_level::nothrow);
static_assert(DefaultFacade::minimum_destructibility == pro::constraint_level::nothrow);
static_assert(DefaultFacade::maximum_size >= 2 * sizeof(void*));
static_assert(DefaultFacade::maximum_alignment >= sizeof(void*));
static_assert(std::is_nothrow_default_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_trivially_default_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::nullptr_t>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, std::nullptr_t>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::in_place_type_t<MockMovablePtr>, int>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::in_place_type_t<MockCopyablePtr>, int>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::in_place_type_t<MockCopyableSmallPtr>, int>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::in_place_type_t<MockTrivialPtr>, int>);

struct RelocatableFacade : pro::facade<> {};
static_assert(!std::is_copy_constructible_v<pro::proxy<RelocatableFacade>>);
static_assert(!std::is_copy_assignable_v<pro::proxy<RelocatableFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<RelocatableFacade>>);
static_assert(!std::is_trivially_move_constructible_v<pro::proxy<RelocatableFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<RelocatableFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<RelocatableFacade>>);
static_assert(std::is_nothrow_destructible_v<pro::proxy<RelocatableFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<RelocatableFacade>>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockMovablePtr>);
static_assert(pro::proxiable<MockMovablePtr, RelocatableFacade>);
static_assert(pro::proxiable<MockCopyablePtr, RelocatableFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, RelocatableFacade>);
static_assert(pro::proxiable<MockTrivialPtr, RelocatableFacade>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockTrivialPtr>);

struct CopyableFacade : pro::facade<> {
  static constexpr auto minimum_copyability = pro::constraint_level::nontrivial;
};
static_assert(std::is_copy_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_nothrow_copy_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_copy_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_nothrow_copy_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_move_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_destructible_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<CopyableFacade>>);
static_assert(!pro::proxiable<MockMovablePtr, CopyableFacade>);
static_assert(pro::proxiable<MockCopyablePtr, CopyableFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, CopyableFacade>);
static_assert(pro::proxiable<MockTrivialPtr, CopyableFacade>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableFacade>, MockMovablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockTrivialPtr>);

struct CopyableSmallFacade : pro::facade<> {
  static constexpr std::size_t maximum_size = sizeof(void*);
  static constexpr auto minimum_copyability = pro::constraint_level::nontrivial;
};
static_assert(!pro::proxiable<MockMovablePtr, CopyableSmallFacade>);
static_assert(!pro::proxiable<MockCopyablePtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, CopyableSmallFacade>);
static_assert(!pro::proxiable<MockTrivialPtr, CopyableSmallFacade>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockMovablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockMovablePtr>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockCopyablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableSmallFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableSmallFacade>, MockCopyableSmallPtr>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockTrivialPtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockTrivialPtr>);

struct TrivialFacade : pro::facade<> {
  static constexpr auto minimum_copyability = pro::constraint_level::trivial;
  static constexpr auto minimum_relocatability = pro::constraint_level::trivial;
  static constexpr auto minimum_destructibility = pro::constraint_level::trivial;
};
static_assert(std::is_trivially_copy_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_copy_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(!std::is_trivially_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_destructible_v<pro::proxy<TrivialFacade>>);
static_assert(!pro::proxiable<MockMovablePtr, TrivialFacade>);
static_assert(!pro::proxiable<MockCopyablePtr, TrivialFacade>);
static_assert(!pro::proxiable<MockCopyableSmallPtr, TrivialFacade>);
static_assert(pro::proxiable<MockTrivialPtr, TrivialFacade>);
static_assert(!std::is_constructible_v<pro::proxy<TrivialFacade>, MockMovablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<TrivialFacade>, MockMovablePtr>);
static_assert(!std::is_constructible_v<pro::proxy<TrivialFacade>, MockCopyablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<TrivialFacade>, MockCopyablePtr>);
static_assert(!std::is_constructible_v<pro::proxy<TrivialFacade>, MockCopyableSmallPtr>);
static_assert(!std::is_assignable_v<pro::proxy<TrivialFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<TrivialFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<TrivialFacade>, MockTrivialPtr>);

enum class LifetimeOperationType {
  kNone,
  kValueConstruction,
  kCopyConstruction,
  kMoveConstruction,
  kDestruction
};

struct LifetimeOperation {
  int Version;
  LifetimeOperationType Operation;

  bool operator==(const LifetimeOperation& rhs) const
      { return Version == rhs.Version && Operation == rhs.Operation; }
};

class LifetimeTracker {
 public:
  LifetimeTracker() : max_version_(0u) {}
  LifetimeTracker(const LifetimeTracker&) = delete;

  class Session {
   public:
    Session(LifetimeTracker* host) : version_(++host->max_version_), host_(host)
        { host_->ops_.push_back({ version_, LifetimeOperationType::kValueConstruction }); }
    Session(const Session& rhs) : version_(++rhs.host_->max_version_), host_(rhs.host_)
        { host_->ops_.push_back({ version_, LifetimeOperationType::kCopyConstruction }); }
    Session(Session&& rhs) noexcept : version_(++rhs.host_->max_version_), host_(rhs.host_)
        { host_->ops_.push_back({ version_, LifetimeOperationType::kMoveConstruction }); }
    ~Session() { host_->ops_.push_back({ version_, LifetimeOperationType::kDestruction }); }

   private:
    int version_;
    LifetimeTracker* const host_;
  };

  Session CreateSession() { return Session{this}; }
  const std::vector<LifetimeOperation>& GetOperations() const { return ops_; }

 private:
  int max_version_;
  std::vector<LifetimeOperation> ops_;
};
}  // namespace

TEST(ProxyLifetimeTests, TestTrivialPtr) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    pro::proxy<TrivialFacade> p1 = &session;
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestUniquePtr) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    pro::proxy<RelocatableFacade> p1 = std::make_unique<LifetimeTracker::Session>(std::move(session));
    expected_ops.push_back({ 2, LifetimeOperationType::kMoveConstruction });
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.push_back({ 2, LifetimeOperationType::kDestruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestUniquePtrBackedByMemoryPool) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    static std::pmr::unsynchronized_pool_resource memory_pool;
    std::pmr::polymorphic_allocator<> alloc{ &memory_pool };
    auto deleter = [alloc](LifetimeTracker::Session* ptr) mutable
        { alloc.delete_object<LifetimeTracker::Session>(ptr); };
    auto instance = alloc.new_object<LifetimeTracker::Session>(std::move(session));
    pro::proxy<RelocatableFacade> p1 = std::unique_ptr<LifetimeTracker::Session, decltype(deleter)>{ instance, deleter };
    expected_ops.push_back({ 2, LifetimeOperationType::kMoveConstruction });
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.push_back({ 2, LifetimeOperationType::kDestruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMakeProxyWithSBO) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    auto p1 = pro::make_proxy<CopyableFacade>(std::move(session));
    expected_ops.push_back({ 2, LifetimeOperationType::kMoveConstruction });
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    expected_ops.push_back({ 3, LifetimeOperationType::kMoveConstruction });
    expected_ops.push_back({ 2, LifetimeOperationType::kDestruction });
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p3 = p2;
    ASSERT_TRUE(p2.has_value());
    ASSERT_TRUE(p3.has_value());
    expected_ops.push_back({ 4, LifetimeOperationType::kCopyConstruction });
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.push_back({ 4, LifetimeOperationType::kDestruction });
  expected_ops.push_back({ 3, LifetimeOperationType::kDestruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMakeProxyWithoutSBO) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    auto p1 = pro::make_proxy<CopyableSmallFacade>(std::move(session));
    expected_ops.push_back({ 2, LifetimeOperationType::kMoveConstruction });
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p3 = p2;
    ASSERT_TRUE(p2.has_value());
    ASSERT_TRUE(p3.has_value());
    expected_ops.push_back({ 3, LifetimeOperationType::kCopyConstruction });
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.push_back({ 3, LifetimeOperationType::kDestruction });
  expected_ops.push_back({ 2, LifetimeOperationType::kDestruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestLifetimeSharedWithSharedPtr) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    pro::proxy<CopyableFacade> p1 = std::make_shared<LifetimeTracker::Session>(std::move(session));
    expected_ops.push_back({ 2, LifetimeOperationType::kMoveConstruction });
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p3 = p2;
    ASSERT_TRUE(p2.has_value());
    ASSERT_TRUE(p3.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.push_back({ 2, LifetimeOperationType::kDestruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}
