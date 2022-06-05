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

struct ConstructionFailure : std::exception {
  ConstructionFailure(LifetimeOperationType operation_type)
      : operation_type_(operation_type) {}

  LifetimeOperationType operation_type_;
};

class LifetimeTracker {
 public:
  LifetimeTracker() = default;
  LifetimeTracker(const LifetimeTracker&) = delete;

  class Session {
   public:
    Session(LifetimeTracker* host)
        : id_(host->AllocateId(LifetimeOperationType::kValueConstruction)),
          host_(host) {}
    Session(std::initializer_list<int>, LifetimeTracker* host)
        : id_(host->AllocateId(LifetimeOperationType::kInitializerListConstruction)),
          host_(host) {}
    Session(const Session& rhs)
        : id_(rhs.host_->AllocateId(LifetimeOperationType::kCopyConstruction)),
          host_(rhs.host_) {}
    Session(Session&& rhs) noexcept :
          id_(rhs.host_->AllocateId(LifetimeOperationType::kMoveConstruction)),
          host_(rhs.host_) {}
    ~Session() { host_->ops_.emplace_back(id_, LifetimeOperationType::kDestruction); }
    Session& operator*() { return *this; }
    friend std::string to_string(const Session& self) { return "Session " + std::to_string(self.id_); }

   private:
    int id_;
    LifetimeTracker* const host_;
  };

  const std::vector<LifetimeOperation>& GetOperations() const { return ops_; }
  void ThrowOnNextConstruction() { throw_on_next_construction_ = true; }

 private:
  int AllocateId(LifetimeOperationType operation_type) {
    if (throw_on_next_construction_) {
      throw_on_next_construction_ = false;
      throw ConstructionFailure{ operation_type };
    }
    ops_.emplace_back(++max_id_, operation_type);
    return max_id_;
  }

  int max_id_ = 0;
  bool throw_on_next_construction_ = false;
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
    // Scenario 1: No exception thrown
    pro::proxy<TestFacade> p1 = LifetimeTracker::Session(&tracker);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 2");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Exception thrown during construction
    LifetimeTracker::Session another_session{ &tracker };
    expected_ops.emplace_back(3, LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<TestFacade> p2 = another_session;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestInPlacePolyConstrction) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    // Scenario 1: No exception thrown
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Exception thrown during construction
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestInPlaceInitializerListPolyConstrction) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    // Scenario 1: No exception thrown
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, { 1, 2, 3 }, &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Exception thrown during construction
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, { 1, 2, 3 }, &tracker };
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestCopyConstrction) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    // Scenario 1: Copy construction from another object that contains a value (no exception thrown)
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
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

    // Scenario 3: Exception thrown during copy construction from another object that contains a value
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      auto p5 = p1;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
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
    // Scenario 1: Move construction from another object that contains a value
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
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
    // Scenario 1: Null assignment to an object that contains a value
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
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

    // Scenario 3: Exception thrown during polymorphic assignment to an object of proxy that does not contain a value
    LifetimeTracker::Session another_session{ &tracker };
    expected_ops.emplace_back(5, LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p2 = another_session;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p2.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 4: Exception thrown during polymorphic assignment to an object of proxy that contains a value
    tracker.ThrowOnNextConstruction();
    exception_thrown = false;
    try {
      p1 = another_session;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 4");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(5, LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(4, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

// TODO: Handle exception
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

    // Scenario 3: Exception thrown during polymorphic assignment to an object of proxy that does not contain a value
    pro::proxy<TestFacade> p2;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p2.emplace<LifetimeTracker::Session>(&tracker);
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p2.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 4: Exception thrown during polymorphic assignment to an object of proxy that contains a value
    tracker.ThrowOnNextConstruction();
    exception_thrown = false;
    try {
      p1.emplace<LifetimeTracker::Session>(&tracker);
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p1.has_value());
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
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

    // Scenario 3: Exception thrown during polymorphic assignment to an object of proxy that does not contain a value
    pro::proxy<TestFacade> p2;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p2.emplace<LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p2.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 4: Exception thrown during polymorphic assignment to an object of proxy that contains a value
    tracker.ThrowOnNextConstruction();
    exception_thrown = false;
    try {
      p1.emplace<LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p1.has_value());
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestCopyAssignment) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    // Scenario 1: Copy assignment from an object that contains a value to another object that does not contain a value
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
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

    // Scenario 7: Exception thrown during copy assignment from an object that contains a value to another object that does not contain a value
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = p2;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 3");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 8: Exception thrown during copy assignment from an object that contains a value to another object that contains a value
    p1 = p2;
    expected_ops.emplace_back(8, LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(9, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(8, LifetimeOperationType::kDestruction);
    tracker.ThrowOnNextConstruction();
    exception_thrown = false;
    try {
      p1 = p2;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.operation_type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 9");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 3");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(9, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyUnitTests, TestMoveAssignment) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    // Scenario 1: Move assignment from an object that contains a value to another object that does not contain a value
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
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
    expected_ops.emplace_back(3, LifetimeOperationType::kValueConstruction);
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
    // Scenario 1: Reset an object of proxy that contains a value
    pro::proxy<TestFacade> p1;
    p1.emplace<LifetimeTracker::Session>(&tracker);
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
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

// TODO
TEST(ProxyUnitTests, TestSwap) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 1: Swap two objects of proxy that both contain a value
    pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    expected_ops.emplace_back(2, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
    swap(p1, p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 4");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 5");
    expected_ops.emplace_back(3, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(4, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(5, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);

    // Scenario 2: Swap two objects of proxy where the first object contains the value and the second does not contain a value
    p2.reset();
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(5, LifetimeOperationType::kDestruction);
    swap(p1, p2);
    // TODO
    /*// Scenario 2: Copy assignment from an object that contains a value to another object that contains a value
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
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);*/
  }
  expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}
