// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include "proxy.h"
#include "utils.h"

namespace {

struct TraitsReflection {
 public:
  template <class P>
  constexpr explicit TraitsReflection(std::in_place_type_t<pro::details::sbo_ptr<P>>)
      : sbo_enabled_(true) {}
  template <class P>
  constexpr explicit TraitsReflection(std::in_place_type_t<pro::details::deep_ptr<P>>)
    : sbo_enabled_(false) {}

  bool sbo_enabled_;
};

struct TestSmallFacade : pro::facade<utils::ToString> {
  using reflection_type = TraitsReflection;
  static constexpr std::size_t maximum_size = sizeof(void*);
  static constexpr auto minimum_copyability = pro::constraint_level::nontrivial;
};
struct TestLargeFacade : pro::facade<utils::ToString> {
  using reflection_type = TraitsReflection;
  static constexpr auto minimum_copyability = pro::constraint_level::nontrivial;
};

}  // namespace

TEST(ProxyCreationTests, TestMakeProxy_WithSBO_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  utils::LifetimeTracker::Session session{ &tracker };
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
  {
    auto p = pro::make_proxy<TestLargeFacade>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 2");
    ASSERT_TRUE(p.reflect().sbo_enabled_);
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
    auto p = pro::make_proxy<TestLargeFacade, utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 1");
    ASSERT_TRUE(p.reflect().sbo_enabled_);
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
    auto p = pro::make_proxy<TestLargeFacade, utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 1");
    ASSERT_TRUE(p.reflect().sbo_enabled_);
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
    auto p1 = pro::make_proxy<TestLargeFacade, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    ASSERT_TRUE(p1.reflect().sbo_enabled_);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    ASSERT_TRUE(p2.reflect().sbo_enabled_);
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
    auto p1 = pro::make_proxy<TestLargeFacade, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    ASSERT_TRUE(p2.reflect().sbo_enabled_);
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
    auto p = pro::make_proxy<TestSmallFacade>(session);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 2");
    ASSERT_FALSE(p.reflect().sbo_enabled_);
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
    auto p = pro::make_proxy<TestSmallFacade, utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 1");
    ASSERT_FALSE(p.reflect().sbo_enabled_);
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
    auto p = pro::make_proxy<TestSmallFacade, utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 1");
    ASSERT_FALSE(p.reflect().sbo_enabled_);
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
    auto p1 = pro::make_proxy<TestSmallFacade, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    ASSERT_FALSE(p1.reflect().sbo_enabled_);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    ASSERT_FALSE(p2.reflect().sbo_enabled_);
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
    auto p1 = pro::make_proxy<TestSmallFacade, utils::LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 1");
    ASSERT_FALSE(p2.reflect().sbo_enabled_);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}
