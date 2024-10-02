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
class IntrusiveImpl : public TestBase {
 public:
  explicit IntrusiveImpl(int seed) noexcept : seed_(seed) {}
  IntrusiveImpl(const IntrusiveImpl&) noexcept = default;
  int Fun() const noexcept override { return seed_ ^ (TypeSeries + 1u); }

 private:
  int seed_;
};

template <std::size_t FromTypeSeries>
void FillProxyTestData(std::vector<pro::proxy<TestFacade>>& data) {
  if constexpr (FromTypeSeries < TypeSeriesCount) {
    for (std::size_t i = FromTypeSeries; i < data.size(); i += TypeSeriesCount) {
      data[i] = pro::make_proxy<TestFacade, NonIntrusiveImpl<FromTypeSeries>>(static_cast<int>(i));
    }
    FillProxyTestData<FromTypeSeries + 1u>(data);
  }
}

std::vector<pro::proxy<TestFacade>> GenerateProxyTestData() {
  std::vector<pro::proxy<TestFacade>> result(TestDataSize);
  FillProxyTestData<0u>(result);
  return result;
}

template <std::size_t FromTypeSeries>
void FillVirtualFunctionTestData(std::vector<std::unique_ptr<TestBase>>& data) {
  if constexpr (FromTypeSeries < TypeSeriesCount) {
    for (std::size_t i = FromTypeSeries; i < data.size(); i += TypeSeriesCount) {
      data[i].reset(new IntrusiveImpl<FromTypeSeries>(static_cast<int>(i)));
    }
    FillVirtualFunctionTestData<FromTypeSeries + 1u>(data);
  }
}

std::vector<std::unique_ptr<TestBase>> GenerateVirtualFunctionTestData() {
  std::vector<std::unique_ptr<TestBase>> result(TestDataSize);
  FillVirtualFunctionTestData<0u>(result);
  return result;
}

}  // namespace

const std::vector<pro::proxy<TestFacade>> ProxyTestData = GenerateProxyTestData();
const std::vector<std::unique_ptr<TestBase>> VirtualFunctionTestData = GenerateVirtualFunctionTestData();
