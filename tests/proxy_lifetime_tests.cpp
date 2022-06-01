// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <vector>
#include <memory>
#include <memory_resource>
#include <gtest/gtest.h>
#include <proxy.h>

namespace {

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
    LifetimeTracker& operator*() const { return *host_; }

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

struct MovableFacade : pro::facade<> {};

static_assert(MovableFacade::minimum_copyability == pro::constraint_level::none);
static_assert(MovableFacade::minimum_relocatability == pro::constraint_level::nothrow);
static_assert(MovableFacade::minimum_destructibility == pro::constraint_level::nothrow);
static_assert(MovableFacade::maximum_size >= 2 * sizeof(void*));
static_assert(MovableFacade::maximum_alignment >= sizeof(void*));
static_assert(std::is_nothrow_default_constructible_v<pro::proxy<MovableFacade>>);
static_assert(!std::is_trivially_default_constructible_v<pro::proxy<MovableFacade>>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<MovableFacade>, std::nullptr_t>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<MovableFacade>, std::nullptr_t>);
static_assert(!std::is_copy_constructible_v<pro::proxy<MovableFacade>>);
static_assert(!std::is_copy_assignable_v<pro::proxy<MovableFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<MovableFacade>>);
static_assert(!std::is_trivially_move_constructible_v<pro::proxy<MovableFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<MovableFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<MovableFacade>>);
static_assert(std::is_nothrow_destructible_v<pro::proxy<MovableFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<MovableFacade>>);

struct CopyableFacade : pro::facade<> {
  static constexpr auto minimum_copyability = pro::constraint_level::nontrivial;
};

static_assert(std::is_nothrow_default_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_default_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, std::nullptr_t>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, std::nullptr_t>);
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

struct TrivialFacade : pro::facade<> {
  static constexpr auto minimum_copyability = pro::constraint_level::trivial;
  static constexpr auto minimum_relocatability = pro::constraint_level::trivial;
  static constexpr auto minimum_destructibility = pro::constraint_level::trivial;
};

static_assert(std::is_nothrow_default_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(!std::is_trivially_default_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<TrivialFacade>, std::nullptr_t>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<TrivialFacade>, std::nullptr_t>);
static_assert(std::is_trivially_copy_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_copy_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(!std::is_trivially_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_destructible_v<pro::proxy<TrivialFacade>>);

}  // namespace

TEST(ProxyLifetimeTests, TestTrivialType) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    pro::proxy<CopyableFacade> p1 = &session;
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestLifetimeExclusiveWithUniquePtr) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    pro::proxy<MovableFacade> p1 = std::make_unique<LifetimeTracker::Session>(std::move(session));
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

TEST(ProxyLifetimeTests, TestLifetimeExclusiveWithUniquePtrBackedByMemoryPool) {
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
    pro::proxy<MovableFacade> p1 = std::unique_ptr<LifetimeTracker::Session, decltype(deleter)>{ instance, deleter };
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

TEST(ProxyLifetimeTests, TestLifetimeExclusiveWithSBO) {
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
