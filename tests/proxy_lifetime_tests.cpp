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
  LifetimeOperation(int id, LifetimeOperationType type) : id_(id), type_(type) {}

  bool operator==(const LifetimeOperation& rhs) const
      { return id_ == rhs.id_ && type_ == rhs.type_; }

  int id_;
  LifetimeOperationType type_;
};

struct ConstructionFailure : std::exception {
  ConstructionFailure(LifetimeOperationType type) : type_(type) {}

  LifetimeOperationType type_;
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

TEST(ProxyLifetimeTests, TestDefaultConstrction) {
  pro::proxy<TestFacade> p;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestNullConstrction) {
  pro::proxy<TestFacade> p = nullptr;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestPolyConstrction) {
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

TEST(ProxyLifetimeTests, TestPolyConstrction_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    LifetimeTracker::Session another_session{ &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<TestFacade> p = another_session;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlacePolyConstrction) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlacePolyConstrction_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlaceInitializerListPolyConstrction) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, { 1, 2, 3 }, &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlaceInitializerListPolyConstrction_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, { 1, 2, 3 }, &tracker };
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyConstrction_FromValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    expected_ops.emplace_back(2, LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyConstrction_FromValue_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      auto p2 = p1;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyConstrction_FromNull) {
  pro::proxy<TestFacade> p1;
  auto p2 = p1;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestMoveConstrction_FromValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    expected_ops.emplace_back(2, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveConstrction_FromNull) {
  pro::proxy<TestFacade> p1;
  auto p2 = std::move(p1);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestNullAssignment_ToValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    p = nullptr;
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
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
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    p = LifetimeTracker::Session{ &tracker };
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 3");
    expected_ops.emplace_back(2, LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_ToValue_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    LifetimeTracker::Session session{ &tracker };
    expected_ops.emplace_back(2, LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p = session;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_ToNull) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p;
    p = LifetimeTracker::Session{ &tracker };
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

TEST(ProxyLifetimeTests, TestPolyAssignment_ToNull_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    LifetimeTracker::Session session{ &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p = session;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlacePolyAssignment_ToValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    p.emplace<LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 2");
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlacePolyAssignment_ToValue_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<LifetimeTracker::Session>(&tracker);
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlacePolyAssignment_ToNull) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p;
    p.emplace<LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlacePolyAssignment_ToNull_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<LifetimeTracker::Session>(&tracker);
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlaceInitializerListPolyAssignment_ToValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    p.emplace<LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 2");
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlaceInitializerListPolyAssignment_ToValue_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlaceInitializerListPolyAssignment_ToNull) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p;
    p.emplace<LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 1");
    expected_ops.emplace_back(1, LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestInPlaceInitializerListPolyAssignment_ToNull_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<LifetimeTracker::Session>({ 1, 2, 3 }, &tracker);
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValueToValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, LifetimeOperationType::kValueConstruction);
    p1 = p2;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 4");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    expected_ops.emplace_back(3, LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(4, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(4, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValueToValue_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = p2;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 1");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValueToSelf) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif  // __clang__
    p = p;
#ifdef __clang__
#pragma clang diagnostic pop
#endif  // __clang__
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(p.invoke(), "Session 3");
    expected_ops.emplace_back(2, LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValueToNull) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1;
    pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    p1 = p2;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 3");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 1");
    expected_ops.emplace_back(2, LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(3, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValueToNull_Exception) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1;
    pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = p2;
    } catch (const ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNullToValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2;
    p1 = p2;
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNullToSelf) {
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

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNullToNull) {
  pro::proxy<TestFacade> p1;
  pro::proxy<TestFacade> p2;
  p1 = p2;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValueToValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, LifetimeOperationType::kValueConstruction);
    p1 = std::move(p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 3");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValueToNull) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1;
    pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    p1 = std::move(p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 2");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(2, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromNullToValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2;
    p1 = std::move(p2);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromNullToNull) {
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
  ASSERT_EQ(p1.invoke(), "123");
}

TEST(ProxyLifetimeTests, TestReset_FromValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    p.reset();
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestReset_FromNull) {
  pro::proxy<TestFacade> p;
  p.reset();
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestSwap_ValueValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(2, LifetimeOperationType::kValueConstruction);
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
  }
  expected_ops.emplace_back(5, LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(4, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_ValueSelf) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    swap(p, p);
    expected_ops.emplace_back(2, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_ValueNull) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    pro::proxy<TestFacade> p2;
    swap(p1, p2);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(p2.invoke(), "Session 2");
    expected_ops.emplace_back(2, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_NullValue) {
  LifetimeTracker tracker;
  std::vector<LifetimeOperation> expected_ops;
  {
    pro::proxy<TestFacade> p1;
    pro::proxy<TestFacade> p2{ std::in_place_type<LifetimeTracker::Session>, &tracker };
    expected_ops.emplace_back(1, LifetimeOperationType::kValueConstruction);
    swap(p1, p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(p1.invoke(), "Session 2");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(2, LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, LifetimeOperationType::kDestruction);
  }
  expected_ops.emplace_back(2, LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_NullSelf) {
  pro::proxy<TestFacade> p;
  swap(p, p);
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestSwap_NullNull) {
  pro::proxy<TestFacade> p1;
  pro::proxy<TestFacade> p2;
  swap(p1, p2);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}
