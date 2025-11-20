// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "proxy_operation_benchmark_context.h"

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
  void* padding_[5]{};
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
  void* padding_[5]{};
  int seed_;
};

template <int V>
struct IntConstant {};

template <int FromTypeSeries, class T, class F>
void FillTestData(std::vector<T>& data, const F& generator) {
  if constexpr (FromTypeSeries < TypeSeriesCount) {
    for (int i = FromTypeSeries; i < TestDataSize; i += TypeSeriesCount) {
      data[i] = generator(IntConstant<FromTypeSeries>{}, i);
    }
    FillTestData<FromTypeSeries + 1>(data, generator);
  }
}

template <class F>
auto GenerateTestData(const F& generator) {
  std::vector<decltype(generator(IntConstant<0>{}, 0))> result(TestDataSize);
  FillTestData<0>(result, generator);
  return result;
}

} // namespace

std::vector<pro::proxy<InvocationTestFacade>>
    GenerateSmallObjectProxyTestData() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return pro::make_proxy<InvocationTestFacade,
                               NonIntrusiveSmallImpl<TypeSeries>>(seed);
      });
}
std::vector<pro::proxy<NothrowRelocatableInvocationTestFacade>>
    GenerateSmallObjectProxyTestData_NothrowRelocatable() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return pro::make_proxy<NothrowRelocatableInvocationTestFacade,
                               NonIntrusiveSmallImpl<TypeSeries>>(seed);
      });
}
std::vector<pro::proxy<InvocationTestFacade>>
    GenerateSmallObjectProxyTestData_Shared() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return pro::make_proxy_shared<InvocationTestFacade,
                                      NonIntrusiveSmallImpl<TypeSeries>>(seed);
      });
}
std::vector<std::unique_ptr<InvocationTestBase>>
    GenerateSmallObjectVirtualFunctionTestData() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return std::unique_ptr<InvocationTestBase>{
            new IntrusiveSmallImpl<TypeSeries>(seed)};
      });
}
std::vector<std::shared_ptr<InvocationTestBase>>
    GenerateSmallObjectVirtualFunctionTestData_Shared() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return std::shared_ptr<InvocationTestBase>{
            std::make_shared<IntrusiveSmallImpl<TypeSeries>>(seed)};
      });
}
std::vector<std::any> GenerateSmallObjectAnyTestData() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return std::make_any<NonIntrusiveSmallImpl<TypeSeries>>(seed);
      });
}

std::vector<pro::proxy<InvocationTestFacade>>
    GenerateLargeObjectProxyTestData() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return pro::make_proxy<InvocationTestFacade,
                               NonIntrusiveLargeImpl<TypeSeries>>(seed);
      });
}
std::vector<pro::proxy<NothrowRelocatableInvocationTestFacade>>
    GenerateLargeObjectProxyTestData_NothrowRelocatable() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return pro::make_proxy<NothrowRelocatableInvocationTestFacade,
                               NonIntrusiveLargeImpl<TypeSeries>>(seed);
      });
}
std::vector<pro::proxy<InvocationTestFacade>>
    GenerateLargeObjectProxyTestData_Shared() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return pro::make_proxy_shared<InvocationTestFacade,
                                      NonIntrusiveLargeImpl<TypeSeries>>(seed);
      });
}
std::vector<std::unique_ptr<InvocationTestBase>>
    GenerateLargeObjectVirtualFunctionTestData() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return std::unique_ptr<InvocationTestBase>{
            new IntrusiveLargeImpl<TypeSeries>(seed)};
      });
}
std::vector<std::shared_ptr<InvocationTestBase>>
    GenerateLargeObjectVirtualFunctionTestData_Shared() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return std::shared_ptr<InvocationTestBase>{
            std::make_shared<IntrusiveLargeImpl<TypeSeries>>(seed)};
      });
}
std::vector<std::any> GenerateLargeObjectAnyTestData() {
  return GenerateTestData(
      []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
        return std::make_any<NonIntrusiveLargeImpl<TypeSeries>>(seed);
      });
}
