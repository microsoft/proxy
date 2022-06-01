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

struct DefaultFacade : pro::facade<> {};
struct CopyableFacade : pro::facade<> {
  static constexpr auto minimum_copyability = pro::constraint_level::nontrivial;
};

}  // namespace

TEST(ProxyTests, TestLifetimeDetached) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    pro::proxy<CopyableFacade> p1 = &session;
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = p1;
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyTests, TestLifetimeExclusiveWithUniquePtr) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    pro::proxy<DefaultFacade> p1 = std::make_unique<LifetimeTracker::Session>(std::move(session));
    expected_ops.push_back({ 2, LifetimeOperationType::kMoveConstruction });
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = std::move(p1);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.push_back({ 2, LifetimeOperationType::kDestruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyTests, TestLifetimeExclusiveWithUniquePtrBackedByMemoryPool) {
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
    pro::proxy<DefaultFacade> p1 = std::unique_ptr<LifetimeTracker::Session, decltype(deleter)>{ instance, deleter };
    expected_ops.push_back({ 2, LifetimeOperationType::kMoveConstruction });
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = std::move(p1);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.push_back({ 2, LifetimeOperationType::kDestruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyTests, TestLifetimeExclusiveWithSBO) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    auto p1 = pro::make_proxy<CopyableFacade>(std::move(session));
    expected_ops.push_back({ 2, LifetimeOperationType::kMoveConstruction });
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = std::move(p1);
    expected_ops.push_back({ 3, LifetimeOperationType::kMoveConstruction });
    expected_ops.push_back({ 2, LifetimeOperationType::kDestruction });
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p3 = p2;
    expected_ops.push_back({ 4, LifetimeOperationType::kCopyConstruction });
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.push_back({ 4, LifetimeOperationType::kDestruction });
  expected_ops.push_back({ 3, LifetimeOperationType::kDestruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyTests, TestLifetimeShareWithSharedPtr) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  auto session = tracker.CreateSession();
  expected_ops.push_back({ 1, LifetimeOperationType::kValueConstruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  {
    pro::proxy<CopyableFacade> p1 = std::make_shared<LifetimeTracker::Session>(std::move(session));
    expected_ops.push_back({ 2, LifetimeOperationType::kMoveConstruction });
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p2 = std::move(p1);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    auto p3 = p2;
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.push_back({ 2, LifetimeOperationType::kDestruction });
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}
