// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include "proxy.h"
#include "utils.h"

namespace proxy_lifetime_tests_details {

struct TestFacade : pro::facade_builder
    ::add_convention<utils::spec::FreeToString, std::string()>
    ::support_relocation<pro::constraint_level::nontrivial>
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_direct_convention<pro::conversion_dispatch<utils::LifetimeTracker::Session>, utils::LifetimeTracker::Session() const&, utils::LifetimeTracker::Session()&&>
    ::build {};

struct TestTrivialFacade : pro::facade_builder
    ::add_facade<utils::spec::Stringable>
    ::support_copy<pro::constraint_level::trivial>
    ::support_relocation<pro::constraint_level::trivial>
    ::support_destruction<pro::constraint_level::trivial>
    ::build {};

struct TestRttiFacade : pro::facade_builder
    ::add_direct_reflection<utils::RttiReflector>
    ::add_facade<TestFacade, true>
    ::build {};

// Additional static asserts for upward conversion
static_assert(!std::is_convertible_v<pro::proxy<TestTrivialFacade>, pro::proxy<utils::spec::Stringable>>);
static_assert(std::is_convertible_v<pro::proxy<TestRttiFacade>, pro::proxy<TestFacade>>);

}  // namespace proxy_lifetime_tests_details

namespace details = proxy_lifetime_tests_details;

TEST(ProxyLifetimeTests, TestDefaultConstrction) {
  pro::proxy<details::TestFacade> p;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestNullConstrction) {
  pro::proxy<details::TestFacade> p = nullptr;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestPolyConstrction_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p = utils::LifetimeTracker::Session(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
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
      pro::proxy<details::TestFacade> p = another_session;
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
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
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
      pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
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
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, { 1, 2, 3 }, &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
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
      pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, { 1, 2, 3 }, &tracker };
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
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
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
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
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
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyConstrction_FromNull) {
  pro::proxy<details::TestFacade> p1;
  auto p2 = p1;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestMoveConstrction_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
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
    pro::proxy<details::TestTrivialFacade> p1 = &session;
    ASSERT_TRUE(p1.has_value());
    auto p2 = std::move(p1);
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveConstrction_FromNull) {
  pro::proxy<details::TestFacade> p1;
  auto p2 = std::move(p1);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestNullAssignment_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p = nullptr;
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestNullAssignment_ToNull) {
  pro::proxy<details::TestFacade> p;
  p = nullptr;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestPolyAssignment_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p = utils::LifetimeTracker::Session{ &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 4");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(4, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(4, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
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
    ASSERT_EQ(ToString(*p), "Session 1");
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
    pro::proxy<details::TestFacade> p;
    p = utils::LifetimeTracker::Session{ &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 3");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_FromValue_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    utils::LifetimeTracker::Session session{ &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p;
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
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
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
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
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
    pro::proxy<details::TestFacade> p;
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
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
    pro::proxy<details::TestFacade> p;
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
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p.emplace<utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
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
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
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
    pro::proxy<details::TestFacade> p;
    p.emplace<utils::LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
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
    pro::proxy<details::TestFacade> p;
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
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    p1 = p2;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 4");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
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
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
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
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
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
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
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
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p1;
    pro::proxy<details::TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p1 = p2;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 3");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
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
    pro::proxy<details::TestFacade> p1;
    pro::proxy<details::TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
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
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNull_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p2;
    p1 = p2;
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNull_ToSelf) {
  pro::proxy<details::TestFacade> p;
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
  pro::proxy<details::TestFacade> p1;
  pro::proxy<details::TestFacade> p2;
  p1 = p2;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    p1 = std::move(p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 3");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = std::move(p2);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kMoveConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p1.has_value());
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToSelf) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
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
    pro::proxy<details::TestFacade> p1;
    pro::proxy<details::TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p1 = std::move(p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 2");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p1;
    pro::proxy<details::TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = std::move(p2);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kMoveConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p1.has_value());
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromNull_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p2;
    p1 = std::move(p2);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromNull_ToSelf) {
  pro::proxy<details::TestFacade> p;
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
  pro::proxy<details::TestFacade> p1;
  pro::proxy<details::TestFacade> p2;
  p1 = std::move(p2);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestHasValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p;
    ASSERT_FALSE(p.has_value());
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestOperatorBool) {
  // Implicit conversion to bool shall be prohibited.
  static_assert(!std::is_nothrow_convertible_v<pro::proxy<details::TestFacade>, bool>);

  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p;
    ASSERT_FALSE(static_cast<bool>(p));
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(static_cast<bool>(p));
    ASSERT_EQ(ToString(*p), "Session 1");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestEqualsToNullptr) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p;
    ASSERT_TRUE(p == nullptr);
    ASSERT_TRUE(nullptr == p);
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p != nullptr);
    ASSERT_TRUE(nullptr != p);
    ASSERT_EQ(ToString(*p), "Session 1");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestReset_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    p.reset();
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestReset_FromNull) {
  pro::proxy<details::TestFacade> p;
  p.reset();
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestSwap_Value_Value) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kValueConstruction);
    swap(p1, p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 4");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 5");
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
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    swap(p, p);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 3");
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
    pro::proxy<details::TestFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p2;
    swap(p1, p2);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
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
    pro::proxy<details::TestFacade> p1;
    pro::proxy<details::TestFacade> p2{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    swap(p1, p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 2");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_Null_Self) {
  pro::proxy<details::TestFacade> p;
  swap(p, p);
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestSwap_Null_Null) {
  pro::proxy<details::TestFacade> p1;
  pro::proxy<details::TestFacade> p2;
  swap(p1, p2);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, Test_DirectConvension_Lvalue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto session = utils::LifetimeTracker::Session{p};
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(to_string(session), "Session 2");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, Test_DirectConvension_Rvalue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    auto session = utils::LifetimeTracker::Session{std::move(p)};
    ASSERT_FALSE(p.has_value());
    ASSERT_EQ(to_string(session), "Session 2");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, Test_DirectConvension_Rvalue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestFacade> p{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      auto session = static_cast<utils::LifetimeTracker::Session>(std::move(p));
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kMoveConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, Test_UpwardCopyConvension_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestRttiFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_STREQ(p1.GetTypeName(), typeid(utils::LifetimeTracker::Session).name());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, Test_UpwardCopyConvension_FromNull) {
  pro::proxy<details::TestRttiFacade> p1;
  pro::proxy<details::TestFacade> p2 = p1;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, Test_UpwardMoveConvension_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<details::TestRttiFacade> p1{ std::in_place_type<utils::LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<details::TestFacade> p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, Test_UpwardMoveConvension_FromNull) {
  pro::proxy<details::TestRttiFacade> p1;
  pro::proxy<details::TestFacade> p2 = std::move(p1);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}
