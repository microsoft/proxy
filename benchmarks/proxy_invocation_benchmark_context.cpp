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
  void* padding_[15]{};
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

}  // namespace

namespace details {

std::pmr::unsynchronized_pool_resource InvocationBenchmarkMemoryPool;

}  // namespace details

const std::vector<pro::proxy<InvocationTestFacade>> SmallObjectInvocationProxyTestData = GenerateTestData(
    []<int TypeSeries>(IntConstant<TypeSeries>, int seed)
        { return pro::make_proxy<InvocationTestFacade, NonIntrusiveSmallImpl<TypeSeries>>(seed); });

const std::vector<std::unique_ptr<InvocationTestBase>> SmallObjectInvocationVirtualFunctionTestData = GenerateTestData(
    []<int TypeSeries>(IntConstant<TypeSeries>, int seed)
        { return std::unique_ptr<InvocationTestBase>{new IntrusiveSmallImpl<TypeSeries>(seed)}; });

const std::vector<std::unique_ptr<InvocationTestBase, details::InvocationBenchmarkPolledDeleter>> PooledSmallObjectInvocationVirtualFunctionTestData = GenerateTestData(
    []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
      return std::unique_ptr<InvocationTestBase, details::InvocationBenchmarkPolledDeleter>{
          std::pmr::polymorphic_allocator<>{&details::InvocationBenchmarkMemoryPool}.new_object<IntrusiveSmallImpl<TypeSeries>>(seed)};
    });

const std::vector<pro::proxy<InvocationTestFacade>> LargeObjectInvocationProxyTestData = GenerateTestData(
    []<int TypeSeries>(IntConstant<TypeSeries>, int seed)
        { return pro::make_proxy<InvocationTestFacade, NonIntrusiveLargeImpl<TypeSeries>>(seed); });

const std::vector<std::unique_ptr<InvocationTestBase>> LargeObjectInvocationVirtualFunctionTestData = GenerateTestData(
    []<int TypeSeries>(IntConstant<TypeSeries>, int seed)
        { return std::unique_ptr<InvocationTestBase>{new IntrusiveLargeImpl<TypeSeries>(seed)}; });

const std::vector<pro::proxy<InvocationTestFacade>> PooledLargeObjectInvocationProxyTestData = GenerateTestData(
    []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
      return pro::allocate_proxy<InvocationTestFacade, NonIntrusiveLargeImpl<TypeSeries>>(
          std::pmr::polymorphic_allocator<>{&details::InvocationBenchmarkMemoryPool}, seed);
    });

const std::vector<std::unique_ptr<InvocationTestBase, details::InvocationBenchmarkPolledDeleter>> PooledLargeObjectInvocationVirtualFunctionTestData = GenerateTestData(
    []<int TypeSeries>(IntConstant<TypeSeries>, int seed) {
      return std::unique_ptr<InvocationTestBase, details::InvocationBenchmarkPolledDeleter>{
          std::pmr::polymorphic_allocator<>{&details::InvocationBenchmarkMemoryPool}.new_object<IntrusiveLargeImpl<TypeSeries>>(seed)};
    });
