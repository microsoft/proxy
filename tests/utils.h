// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _MSFT_PROXY_TEST_UTILS_
#define _MSFT_PROXY_TEST_UTILS_

#include <string>
#include <vector>

namespace utils {

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
    const Session* operator->() const { return this; }
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

namespace spec {

using std::to_string;
PRO_DEF_FREE_DISPATCH(ToString, to_string, std::string());

}  // namespace spec

}  // namespace utils

#endif  // _MSFT_PROXY_TEST_UTILS_
