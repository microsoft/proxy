// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <any>
#include <array>
#include <memory_resource>
#include <mutex>
#include <string>

#include <benchmark/benchmark.h>

#include "proxy.h"

namespace {

constexpr int TestManagedObjectCount = 12000;
constexpr int TypeSeriesCount = 3;

using SmallObject1 = int;
using SmallObject2 = std::shared_ptr<int>;
struct SmallObject3 {
  SmallObject3() noexcept = default;
  SmallObject3(SmallObject3&&) noexcept = default;
  SmallObject3(const SmallObject3&) { throw std::runtime_error{ "Not implemented" }; }

  std::unique_lock<std::mutex> Field1;
};

using LargeObject1 = std::array<char, 100>;
using LargeObject2 = std::array<std::string, 3>;
struct LargeObject3 {
  SmallObject3 Field1;
  void* Padding[15];
};

template <class T>
std::pmr::unsynchronized_pool_resource CreateMemoryPool() {
  return std::pmr::unsynchronized_pool_resource{std::pmr::pool_options{.max_blocks_per_chunk = 1000, .largest_required_pool_block = sizeof(T)}};
}

}  // namespace

struct DefaultFacade : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

void BM_SmallObjectManagementWithProxy(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<pro::proxy<DefaultFacade>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      auto p1 = pro::make_proxy<DefaultFacade, SmallObject1>();
      auto p2 = pro::make_proxy<DefaultFacade, SmallObject2>();
      auto p3 = pro::make_proxy<DefaultFacade, SmallObject3>();

      benchmark::DoNotOptimize(p1);
      benchmark::DoNotOptimize(p2);
      benchmark::DoNotOptimize(p3);

      data.push_back(std::move(p1));
      data.push_back(std::move(p2));
      data.push_back(std::move(p3));
    }
  }
}

void BM_SmallObjectManagementWithSharedPtr(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::shared_ptr<void>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      std::shared_ptr<void> p1 = std::make_shared<SmallObject1>();
      std::shared_ptr<void> p2 = std::make_shared<SmallObject2>();
      std::shared_ptr<void> p3 = std::make_shared<SmallObject3>();

      benchmark::DoNotOptimize(p1);
      benchmark::DoNotOptimize(p2);
      benchmark::DoNotOptimize(p3);

      data.push_back(std::move(p1));
      data.push_back(std::move(p2));
      data.push_back(std::move(p3));
    }
  }
}

void BM_SmallObjectManagementWithSharedPtr_Pooled(benchmark::State& state) {
  auto pool1 = CreateMemoryPool<SmallObject1>();
  auto pool2 = CreateMemoryPool<SmallObject2>();
  auto pool3 = CreateMemoryPool<SmallObject3>();
  for (auto _ : state) {
    std::vector<std::shared_ptr<void>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      std::shared_ptr<void> p1 = std::allocate_shared<SmallObject1>(std::pmr::polymorphic_allocator<>{&pool1});
      std::shared_ptr<void> p2 = std::allocate_shared<SmallObject2>(std::pmr::polymorphic_allocator<>{&pool2});
      std::shared_ptr<void> p3 = std::allocate_shared<SmallObject3>(std::pmr::polymorphic_allocator<>{&pool3});

      benchmark::DoNotOptimize(p1);
      benchmark::DoNotOptimize(p2);
      benchmark::DoNotOptimize(p3);

      data.push_back(std::move(p1));
      data.push_back(std::move(p2));
      data.push_back(std::move(p3));
    }
  }
}

void BM_SmallObjectManagementWithAny(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::any> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      std::any a1 = SmallObject1{};
      std::any a2 = SmallObject2{};
      std::any a3 = SmallObject3{};

      benchmark::DoNotOptimize(a1);
      benchmark::DoNotOptimize(a2);
      benchmark::DoNotOptimize(a3);

      data.push_back(std::move(a1));
      data.push_back(std::move(a2));
      data.push_back(std::move(a3));
    }
  }
}

void BM_LargeObjectManagementWithProxy(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<pro::proxy<DefaultFacade>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      auto p1 = pro::make_proxy<DefaultFacade, LargeObject1>();
      auto p2 = pro::make_proxy<DefaultFacade, LargeObject2>();
      auto p3 = pro::make_proxy<DefaultFacade, LargeObject3>();

      benchmark::DoNotOptimize(p1);
      benchmark::DoNotOptimize(p2);
      benchmark::DoNotOptimize(p3);

      data.push_back(std::move(p1));
      data.push_back(std::move(p2));
      data.push_back(std::move(p3));
    }
  }
}

void BM_LargeObjectManagementWithProxy_Pooled(benchmark::State& state) {
  auto pool1 = CreateMemoryPool<LargeObject1>();
  auto pool2 = CreateMemoryPool<LargeObject2>();
  auto pool3 = CreateMemoryPool<LargeObject3>();
  for (auto _ : state) {
    std::vector<pro::proxy<DefaultFacade>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      auto p1 = pro::allocate_proxy<DefaultFacade, LargeObject1>(std::pmr::polymorphic_allocator<>{&pool1});
      auto p2 = pro::allocate_proxy<DefaultFacade, LargeObject2>(std::pmr::polymorphic_allocator<>{&pool2});
      auto p3 = pro::allocate_proxy<DefaultFacade, LargeObject3>(std::pmr::polymorphic_allocator<>{&pool3});

      benchmark::DoNotOptimize(p1);
      benchmark::DoNotOptimize(p2);
      benchmark::DoNotOptimize(p3);

      data.push_back(std::move(p1));
      data.push_back(std::move(p2));
      data.push_back(std::move(p3));
    }
  }
}

void BM_LargeObjectManagementWithSharedPtr(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::shared_ptr<void>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      std::shared_ptr<void> p1 = std::make_shared<LargeObject1>();
      std::shared_ptr<void> p2 = std::make_shared<LargeObject2>();
      std::shared_ptr<void> p3 = std::make_shared<LargeObject3>();

      benchmark::DoNotOptimize(p1);
      benchmark::DoNotOptimize(p2);
      benchmark::DoNotOptimize(p3);

      data.push_back(std::move(p1));
      data.push_back(std::move(p2));
      data.push_back(std::move(p3));
    }
  }
}

void BM_LargeObjectManagementWithSharedPtr_Pooled(benchmark::State& state) {
  auto pool1 = CreateMemoryPool<LargeObject1>();
  auto pool2 = CreateMemoryPool<LargeObject2>();
  auto pool3 = CreateMemoryPool<LargeObject3>();
  for (auto _ : state) {
    std::vector<std::shared_ptr<void>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      std::shared_ptr<void> p1 = std::allocate_shared<LargeObject1>(std::pmr::polymorphic_allocator<>{&pool1});
      std::shared_ptr<void> p2 = std::allocate_shared<LargeObject2>(std::pmr::polymorphic_allocator<>{&pool2});
      std::shared_ptr<void> p3 = std::allocate_shared<LargeObject3>(std::pmr::polymorphic_allocator<>{&pool3});

      benchmark::DoNotOptimize(p1);
      benchmark::DoNotOptimize(p2);
      benchmark::DoNotOptimize(p3);

      data.push_back(std::move(p1));
      data.push_back(std::move(p2));
      data.push_back(std::move(p3));
    }
  }
}

void BM_LargeObjectManagementWithAny(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::any> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      std::any a1 = LargeObject1{};
      std::any a2 = LargeObject2{};
      std::any a3 = LargeObject3{};

      benchmark::DoNotOptimize(a1);
      benchmark::DoNotOptimize(a2);
      benchmark::DoNotOptimize(a3);

      data.push_back(std::move(a1));
      data.push_back(std::move(a2));
      data.push_back(std::move(a3));
    }
  }
}

BENCHMARK(BM_SmallObjectManagementWithProxy);
BENCHMARK(BM_SmallObjectManagementWithSharedPtr);
BENCHMARK(BM_SmallObjectManagementWithSharedPtr_Pooled);
BENCHMARK(BM_SmallObjectManagementWithAny);
BENCHMARK(BM_LargeObjectManagementWithProxy);
BENCHMARK(BM_LargeObjectManagementWithProxy_Pooled);
BENCHMARK(BM_LargeObjectManagementWithSharedPtr);
BENCHMARK(BM_LargeObjectManagementWithSharedPtr_Pooled);
BENCHMARK(BM_LargeObjectManagementWithAny);
