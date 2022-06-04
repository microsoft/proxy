// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "proxy.h"

namespace {

struct ToString : pro::dispatch<std::string()> {
  template <class T>
  std::string operator()(const T& self) {
    using std::to_string;
    return to_string(self);
  }
};
struct TestFacade : pro::facade<ToString> {
  static constexpr auto minimum_copyability = pro::constraint_level::nontrivial;
};

enum class LifetimeOperationType {
  kNone,
  kValueConstruction,
  kInitializerListConstruction,
  kCopyConstruction,
  kMoveConstruction,
  kDestruction
};

struct LifetimeOperation {
  LifetimeOperation(int id, LifetimeOperationType operation) : id_(id), operation_(operation) {}

  bool operator==(const LifetimeOperation& rhs) const
      { return id_ == rhs.id_ && operation_ == rhs.operation_; }

  int id_;
  LifetimeOperationType operation_;
};

class LifetimeTracker {
 public:
  LifetimeTracker() : max_id_(0u) {}
  LifetimeTracker(const LifetimeTracker&) = delete;

  class Session {
   public:
    Session(LifetimeTracker* host) : id_(++host->max_id_), host_(host)
        { host_->ops_.emplace_back(id_, LifetimeOperationType::kValueConstruction); }
    Session(std::initializer_list<int>, LifetimeTracker* host) : id_(++host->max_id_), host_(host)
        { host_->ops_.emplace_back(id_, LifetimeOperationType::kInitializerListConstruction); }
    Session(const Session& rhs) : id_(++rhs.host_->max_id_), host_(rhs.host_)
        { host_->ops_.emplace_back(id_, LifetimeOperationType::kCopyConstruction); }
    Session(Session&& rhs) noexcept : id_(++rhs.host_->max_id_), host_(rhs.host_)
        { host_->ops_.emplace_back(id_, LifetimeOperationType::kMoveConstruction); }
    ~Session() { host_->ops_.emplace_back(id_, LifetimeOperationType::kDestruction); }
    Session& operator*() { return *this; }
    friend std::string to_string(const Session& self) { return "Session " + std::to_string(self.id_); }

   private:
    int id_;
    LifetimeTracker* const host_;
  };

  const std::vector<LifetimeOperation>& GetOperations() const { return ops_; }

 private:
  int max_id_;
  std::vector<LifetimeOperation> ops_;
};

}  // namespace

TEST(ProxyUnitTests, TestDefaultConstrction) {
  pro::proxy<TestFacade> p;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyUnitTests, TestNullConstrction) {
  pro::proxy<TestFacade> p = nullptr;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyUnitTests, TestPolyConstrction) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p = LifetimeTracker::Session(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 2");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestInPlacePolyConstrction) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestInPlaceInitializerListPolyConstrction) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, { 1, 2, 3 }, &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestCopyConstrction) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 1: Copy construction from another object that contains a value
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    expected_ops.emplace_back(2, LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Copy construction from another object that does not contain a value
    pro::proxy<TestFacade> p3;
    auto p4 = p3;
    ASSERT_FALSE(p3.has_value());
    ASSERT_FALSE(p4.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestMoveConstrction) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 1: Move construction from another object that contains a value
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    expected_ops.emplace_back(2, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Move construction from another object that does not contain a value
    pro::proxy<TestFacade> p3;
    auto p4 = std::move(p3);
    ASSERT_FALSE(p3.has_value());
    ASSERT_FALSE(p4.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestNullAssignment) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 1: Null assignment to an object that contains a value
    p1 = nullptr;
    ASSERT_FALSE(p1.has_value());
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Null assignment to an object that does not contain a value
    p1 = nullptr;
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestPolyAssignment) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    // Scenario 1: Polymorphic assignment to an object of proxy that does not contain a value
    pro::proxy<TestFacade> p1;
    p1 = LifetimeTracker::Session{ &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 2");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Polymorphic assignment to an object of proxy that contains a value
    p1 = LifetimeTracker::Session{ &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 4");
    expected_ops.emplace_back(3, LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(4, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(4, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestInPlacePolyAssignment) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    // Scenario 1: Polymorphic assignment to an object of proxy that does not contain a value
    pro::proxy<TestFacade> p1;
    p1.emplace<LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Polymorphic assignment to an object of proxy that contains a value
    p1.emplace<LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 2");
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestInPlaceInitializerListPolyAssignment) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    // Scenario 1: Polymorphic assignment to an object of proxy that does not contain a value
    pro::proxy<TestFacade> p1;
    p1.emplace<LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Polymorphic assignment to an object of proxy that contains a value
    p1.emplace<LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 2");
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestCopyAssignment) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 1: Copy assignment from an object that contains a value to another object that does not contain a value
    pro::proxy<TestFacade> p2;
    p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 3");
    expected_ops.emplace_back(2, LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(3, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Copy assignment from an object that contains a value to another object that contains a value
    p1 = p2;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 5");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 3");
    expected_ops.emplace_back(4, LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(5, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(4, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 3: Copy assignment from an object that does not contain a value to itself
    pro::proxy<TestFacade> p3;
    p3 = p3;
    ASSERT_FALSE(p3.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 4: Copy assignment from an object that contains a value to itself
    p1 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 7");
    expected_ops.emplace_back(6, LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(5, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(7, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(6, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 5: Copy assignment from an object that does not contain a value to another object that contains a value
    p1 = p3;
    ASSERT_FALSE(p1.has_value());
    ASSERT_FALSE(p3.has_value());
    expected_ops.emplace_back(7, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 6: Copy assignment from an object that does not contain a value to another object that does not contain a value
    p1 = p3;
    ASSERT_FALSE(p1.has_value());
    ASSERT_FALSE(p3.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestMoveAssignment) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 1: Move assignment from an object that contains a value to another object that does not contain a value
    pro::proxy<TestFacade> p2;
    p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    expected_ops.emplace_back(2, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Move assignment from an object that contains a value to another object that contains a value
    pro::proxy<TestFacade> p3{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p3.has_value());
    ASSERT_EQ(p3.invoke(), "Session 3");
    expected_ops.emplace_back(3, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    p2 = std::move(p3);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 4");
    ASSERT_FALSE(p3.has_value());
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(4, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 3: Move assignment from an object that does not contain a value to itself
    p1 = std::move(p1);
    ASSERT_FALSE(p1.has_value());

    // Scenario 4: Move assignment from an object that contains a value to itself
    p2 = std::move(p2);
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 4");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 5: Move assignment from an object that does not contain a value to another object that contains a value
    p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(4, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 6: Move assignment from an object that does not contain a value to another object that does not contain a value
    p1 = std::move(p2);
    ASSERT_FALSE(p1.has_value());
    ASSERT_FALSE(p2.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestHasValue) {
  int foo = 123;
  pro::proxy<TestFacade> p1;
  ASSERT_FALSE(p1.has_value());
  p1 = &foo;
  ASSERT_TRUE(p1.has_value());
  ASSERT_EQ(p1.invoke(), "123");
}

TEST(ProxyUnitTests, TestReset) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1;
    p1.emplace<LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 1: Reset an object of proxy that contains a value
    p1.reset();
    ASSERT_FALSE(p1.has_value());
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Reset an object of proxy that does not contain a value
    p1.reset();
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}
