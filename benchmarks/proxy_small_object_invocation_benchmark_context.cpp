// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "proxy_small_object_invocation_benchmark_context.h"

namespace {

constexpr std::size_t TestDataSize = 1500000;
constexpr std::size_t TypeSeriesCount = 3;

template <std::size_t TypeSeries>
class NonIntrusiveImpl {
 public:
  explicit NonIntrusiveImpl(int seed) noexcept : seed_(seed) {}
  NonIntrusiveImpl(const NonIntrusiveImpl&) noexcept = default;
  int Fun() const noexcept { return seed_ ^ (TypeSeries + 1u); }

 private:
  int seed_;
};

template <std::size_t TypeSeries>
class NonIntrusiveLargeImpl {
 public:
  explicit NonIntrusiveLargeImpl(int seed) noexcept : seed_(seed) {}
  NonIntrusiveLargeImpl(const NonIntrusiveLargeImpl&) noexcept = default;
  int Fun() const noexcept { return seed_ ^ (TypeSeries + 1u); }

 private:
  void* padding_[16];
  int seed_;
};

template <std::size_t TypeSeries>
class IntrusiveImpl : public TestBase {
 public:
  explicit IntrusiveImpl(int seed) noexcept : seed_(seed) {}
  IntrusiveImpl(const IntrusiveImpl&) noexcept = default;
  int Fun() const noexcept override { return seed_ ^ (TypeSeries + 1u); }

 private:
  int seed_;
};

template <std::size_t TypeSeries>
class IntrusiveLargeImpl : public TestBase {
 public:
  explicit IntrusiveLargeImpl(int seed) noexcept : seed_(seed) {}
  IntrusiveLargeImpl(const IntrusiveLargeImpl&) noexcept = default;
  int Fun() const noexcept override { return seed_ ^ (TypeSeries + 1u); }

 private:
  void* padding_[16];
  int seed_;
};

template <template <std::size_t> class T, std::size_t FromTypeSeries>
void FillProxyTestData(std::vector<pro::proxy<TestFacade>>& data) {
  if constexpr (FromTypeSeries < TypeSeriesCount) {
    for (std::size_t i = FromTypeSeries; i < data.size(); i += TypeSeriesCount) {
      data[i] = pro::make_proxy<TestFacade, T<FromTypeSeries>>(static_cast<int>(i));
    }
    FillProxyTestData<T, FromTypeSeries + 1u>(data);
  }
}

template <template <std::size_t> class T>
std::vector<pro::proxy<TestFacade>> GenerateProxyTestData() {
  std::vector<pro::proxy<TestFacade>> result(TestDataSize);
  FillProxyTestData<T, 0u>(result);
  return result;
}

template <template <std::size_t> class T, std::size_t FromTypeSeries>
void FillVirtualFunctionTestData(std::vector<std::unique_ptr<TestBase>>& data) {
  if constexpr (FromTypeSeries < TypeSeriesCount) {
    for (std::size_t i = FromTypeSeries; i < data.size(); i += TypeSeriesCount) {
      data[i].reset(new T<FromTypeSeries>(static_cast<int>(i)));
    }
    FillVirtualFunctionTestData<T, FromTypeSeries + 1u>(data);
  }
}

template <template <std::size_t> class T>
std::vector<std::unique_ptr<TestBase>> GenerateVirtualFunctionTestData() {
  std::vector<std::unique_ptr<TestBase>> result(TestDataSize);
  FillVirtualFunctionTestData<T, 0u>(result);
  return result;
}

}  // namespace

const std::vector<pro::proxy<TestFacade>> ProxyTestData = GenerateProxyTestData<NonIntrusiveImpl>();
const std::vector<std::unique_ptr<TestBase>> VirtualFunctionTestData = GenerateVirtualFunctionTestData<IntrusiveImpl>();
const std::vector<pro::proxy<TestFacade>> ProxyTestDataLarge = GenerateProxyTestData<NonIntrusiveLargeImpl>();
const std::vector<std::unique_ptr<TestBase>> VirtualFunctionTestDataLarge = GenerateVirtualFunctionTestData<IntrusiveLargeImpl>();
