// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "proxy_invocation_benchmark_context.h"

namespace {

constexpr int TestDataSize = 1000000;
constexpr int TypeSeriesCount = 100;

template <int TypeSeries>
class NonIntrusiveSmallImpl {
 public:
  explicit NonIntrusiveSmallImpl(int seed) noexcept : seed_(seed) {}
  NonIntrusiveSmallImpl(const NonIntrusiveSmallImpl&) noexcept = default;
  int Fun() const noexcept { return seed_ ^ (TypeSeries + 1); }

 private:
  int seed_;
};

template <int TypeSeries>
class NonIntrusiveLargeImpl {
 public:
  explicit NonIntrusiveLargeImpl(int seed) noexcept : seed_(seed) {}
  NonIntrusiveLargeImpl(const NonIntrusiveLargeImpl&) noexcept = default;
  int Fun() const noexcept { return seed_ ^ (TypeSeries + 1); }

 private:
  void* padding_[16]{};
  int seed_;
};

template <int TypeSeries>
class IntrusiveSmallImpl : public InvocationTestBase {
 public:
  explicit IntrusiveSmallImpl(int seed) noexcept : seed_(seed) {}
  IntrusiveSmallImpl(const IntrusiveSmallImpl&) noexcept = default;
  int Fun() const noexcept override { return seed_ ^ (TypeSeries + 1); }

 private:
  int seed_;
};

template <int TypeSeries>
class IntrusiveLargeImpl : public InvocationTestBase {
 public:
  explicit IntrusiveLargeImpl(int seed) noexcept : seed_(seed) {}
  IntrusiveLargeImpl(const IntrusiveLargeImpl&) noexcept = default;
  int Fun() const noexcept override { return seed_ ^ (TypeSeries + 1); }

 private:
  void* padding_[16]{};
  int seed_;
};

template <template <int> class T, int FromTypeSeries>
void FillProxyTestData(std::vector<pro::proxy<InvocationTestFacade>>& data) {
  if constexpr (FromTypeSeries < TypeSeriesCount) {
    for (int i = FromTypeSeries; i < TestDataSize; i += TypeSeriesCount) {
      data[i] = pro::make_proxy<InvocationTestFacade, T<FromTypeSeries>>(static_cast<int>(i));
    }
    FillProxyTestData<T, FromTypeSeries + 1>(data);
  }
}

template <template <int> class T>
std::vector<pro::proxy<InvocationTestFacade>> GenerateProxyTestData() {
  std::vector<pro::proxy<InvocationTestFacade>> result(TestDataSize);
  FillProxyTestData<T, 0>(result);
  return result;
}

template <template <int> class T, int FromTypeSeries>
void FillVirtualFunctionTestData(std::vector<std::unique_ptr<InvocationTestBase>>& data) {
  if constexpr (FromTypeSeries < TypeSeriesCount) {
    for (int i = FromTypeSeries; i < TestDataSize; i += TypeSeriesCount) {
      data[i].reset(new T<FromTypeSeries>(static_cast<int>(i)));
    }
    FillVirtualFunctionTestData<T, FromTypeSeries + 1>(data);
  }
}

template <template <int> class T>
std::vector<std::unique_ptr<InvocationTestBase>> GenerateVirtualFunctionTestData() {
  std::vector<std::unique_ptr<InvocationTestBase>> result(TestDataSize);
  FillVirtualFunctionTestData<T, 0>(result);
  return result;
}

}  // namespace

const std::vector<pro::proxy<InvocationTestFacade>> SmallObjectInvocationProxyTestData = GenerateProxyTestData<NonIntrusiveSmallImpl>();
const std::vector<std::unique_ptr<InvocationTestBase>> SmallObjectInvocationVirtualFunctionTestData = GenerateVirtualFunctionTestData<IntrusiveSmallImpl>();
const std::vector<pro::proxy<InvocationTestFacade>> LargeObjectInvocationProxyTestData = GenerateProxyTestData<NonIntrusiveLargeImpl>();
const std::vector<std::unique_ptr<InvocationTestBase>> LargeObjectInvocationVirtualFunctionTestData = GenerateVirtualFunctionTestData<IntrusiveLargeImpl>();
