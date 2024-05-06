// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include "proxy.h"
#include "utils.h"

namespace {

PRO_DEF_FACADE(TestFacade, utils::spec::ToString, pro::copyable_ptr_constraints);
PRO_DEF_FACADE(TestTrivialFacade, utils::spec::ToString, pro::trivial_ptr_constraints);

}  // namespace

TEST(ProxyLifetimeTests, TestDefaultConstrction) {
  pro::proxy<TestFacade> p;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestNullConstrction) {
  pro::proxy<TestFacade> p = nullptr;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestPolyConstrction_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p = utils::LifetimeTracker::Session(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 2");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyConstrction_FromValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    utils::LifetimeTracker::Session another_session{ &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<TestFacade> p = another_session;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyConstrction_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyConstrction_InPlace_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyConstrction_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, { 1, 2, 3 }, &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyConstrction_InPlaceInitializerList_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, { 1, 2, 3 }, &tracker };
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyConstrction_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 1");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyConstrction_FromValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      auto p2 = p1;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyConstrction_FromNull) {
  pro::proxy<TestFacade> p1;
  auto p2 = p1;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestMoveConstrction_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveConstrction_FromValue_Trivial) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    utils::LifetimeTracker::Session session{ &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<TestTrivialFacade> p1 = &session;
    ASSERT_TRUE(p1.has_value());
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveConstrction_FromNull) {
  pro::proxy<TestFacade> p1;
  auto p2 = std::move(p1);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestNullAssignment_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p = nullptr;
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestNullAssignment_ToNull) {
  pro::proxy<TestFacade> p;
  p = nullptr;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestPolyAssignment_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p = utils::LifetimeTracker::Session{ &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 3");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    utils::LifetimeTracker::Session session{ &tracker };
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p = session;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_FromValue_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p;
    p = utils::LifetimeTracker::Session{ &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 2");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_FromValue_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    utils::LifetimeTracker::Session session{ &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p = session;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlace_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 2");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlace_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<utils::LifetimeTracker::Session>(&tracker);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlace_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p;
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlace_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<utils::LifetimeTracker::Session>(&tracker);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlaceInitializerList_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p.emplace<utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 2");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlaceInitializerList_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlaceInitializerList_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p;
    p.emplace<utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 1");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlaceInitializerList_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    p1 = p2;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 4");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(4, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(4, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = p2;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 1");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToSelf) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif  // __clang__
    p = p;
#ifdef __clang__
#pragma clang diagnostic pop
#endif  // __clang__
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 3");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1;
    pro::proxy<TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p1 = p2;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 3");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 1");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1;
    pro::proxy<TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = p2;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNull_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2;
    p1 = p2;
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNull_ToSelf) {
  pro::proxy<TestFacade> p;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif  // __clang__
  p = p;
#ifdef __clang__
#pragma clang diagnostic pop
#endif  // __clang__
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNull_ToNull) {
  pro::proxy<TestFacade> p1;
  pro::proxy<TestFacade> p2;
  p1 = p2;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    p1 = std::move(p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 3");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToSelf) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#elif defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif  // __clang__
    p = std::move(p);
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic pop
#endif  // __clang__
    ASSERT_TRUE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1;
    pro::proxy<TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p1 = std::move(p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 2");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromNull_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2;
    p1 = std::move(p2);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromNull_ToSelf) {
  pro::proxy<TestFacade> p;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#elif defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif  // __clang__
  p = std::move(p);
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic pop
#endif  // __clang__
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromNull_ToNull) {
  pro::proxy<TestFacade> p1;
  pro::proxy<TestFacade> p2;
  p1 = std::move(p2);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestHasValue) {
  int foo = 123;
  pro::proxy<TestFacade> p1;
  ASSERT_FALSE(p1.has_value());
  p1 = &foo;
  ASSERT_TRUE(p1.has_value());
  ASSERT_EQ(p1.ToString(), "123");
}

TEST(ProxyLifetimeTests, TestReset_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p.reset();
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestReset_FromNull) {
  pro::proxy<TestFacade> p;
  p.reset();
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestSwap_Value_Value) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    swap(p1, p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 4");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 5");
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(4, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(5, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(5, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(4, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_Value_Self) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    swap(p, p);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.ToString(), "Session 3");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_Value_Null) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2;
    swap(p1, p2);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.ToString(), "Session 2");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_Null_Value) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1;
    pro::proxy<TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    swap(p1, p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.ToString(), "Session 2");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_Null_Self) {
  pro::proxy<TestFacade> p;
  swap(p, p);
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestSwap_Null_Null) {
  pro::proxy<TestFacade> p1;
  pro::proxy<TestFacade> p2;
  swap(p1, p2);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}
