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

constexpr int TestManagedObjectCount = 600000;
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

struct PolymorphicObjectBase {
  virtual ~PolymorphicObjectBase() = default;
};
template <class T>
struct PolymorphicObject : PolymorphicObjectBase {
  T Value;
};

struct DefaultFacade : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

void BM_SmallObjectManagementWithProxy(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<pro::proxy<DefaultFacade>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.push_back(pro::make_proxy<DefaultFacade, SmallObject1>());
      data.push_back(pro::make_proxy<DefaultFacade, SmallObject2>());
      data.push_back(pro::make_proxy<DefaultFacade, SmallObject3>());
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_SmallObjectManagementWithUniquePtr(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::unique_ptr<PolymorphicObjectBase>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.push_back(std::unique_ptr<PolymorphicObjectBase>{new PolymorphicObject<SmallObject1>()});
      data.push_back(std::unique_ptr<PolymorphicObjectBase>{new PolymorphicObject<SmallObject2>()});
      data.push_back(std::unique_ptr<PolymorphicObjectBase>{new PolymorphicObject<SmallObject3>()});
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_SmallObjectManagementWithSharedPtr(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::shared_ptr<void>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.emplace_back(std::make_shared<SmallObject1>());
      data.emplace_back(std::make_shared<SmallObject2>());
      data.emplace_back(std::make_shared<SmallObject3>());
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_SmallObjectManagementWithSharedPtr_Pooled(benchmark::State& state) {
  static std::pmr::unsynchronized_pool_resource pool;
  std::pmr::polymorphic_allocator<> alloc{&pool};
  for (auto _ : state) {
    std::vector<std::shared_ptr<void>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.emplace_back(std::allocate_shared<SmallObject1>(alloc));
      data.emplace_back(std::allocate_shared<SmallObject2>(alloc));
      data.emplace_back(std::allocate_shared<SmallObject3>(alloc));
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_SmallObjectManagementWithAny(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::any> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.emplace_back(SmallObject1{});
      data.emplace_back(SmallObject2{});
      data.emplace_back(SmallObject3{});
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithProxy(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<pro::proxy<DefaultFacade>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.push_back(pro::make_proxy<DefaultFacade, LargeObject1>());
      data.push_back(pro::make_proxy<DefaultFacade, LargeObject2>());
      data.push_back(pro::make_proxy<DefaultFacade, LargeObject3>());
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithProxy_Pooled(benchmark::State& state) {
  static std::pmr::unsynchronized_pool_resource pool;
  std::pmr::polymorphic_allocator<> alloc{&pool};
  for (auto _ : state) {
    std::vector<pro::proxy<DefaultFacade>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.push_back(pro::allocate_proxy<DefaultFacade, LargeObject1>(alloc));
      data.push_back(pro::allocate_proxy<DefaultFacade, LargeObject2>(alloc));
      data.push_back(pro::allocate_proxy<DefaultFacade, LargeObject3>(alloc));
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithUniquePtr(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::unique_ptr<PolymorphicObjectBase>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.push_back(std::unique_ptr<PolymorphicObjectBase>{new PolymorphicObject<LargeObject1>()});
      data.push_back(std::unique_ptr<PolymorphicObjectBase>{new PolymorphicObject<LargeObject2>()});
      data.push_back(std::unique_ptr<PolymorphicObjectBase>{new PolymorphicObject<LargeObject3>()});
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithSharedPtr(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::shared_ptr<void>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.emplace_back(std::make_shared<LargeObject1>());
      data.emplace_back(std::make_shared<LargeObject2>());
      data.emplace_back(std::make_shared<LargeObject3>());
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithSharedPtr_Pooled(benchmark::State& state) {
  static std::pmr::unsynchronized_pool_resource pool;
  std::pmr::polymorphic_allocator<> alloc{&pool};
  for (auto _ : state) {
    std::vector<std::shared_ptr<void>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.emplace_back(std::allocate_shared<LargeObject1>(alloc));
      data.emplace_back(std::allocate_shared<LargeObject2>(alloc));
      data.emplace_back(std::allocate_shared<LargeObject3>(alloc));
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithAny(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::any> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += TypeSeriesCount) {
      data.emplace_back(LargeObject1{});
      data.emplace_back(LargeObject2{});
      data.emplace_back(LargeObject3{});
    }
    benchmark::DoNotOptimize(data);
  }
}

BENCHMARK(BM_SmallObjectManagementWithProxy);
BENCHMARK(BM_SmallObjectManagementWithUniquePtr);
BENCHMARK(BM_SmallObjectManagementWithSharedPtr);
BENCHMARK(BM_SmallObjectManagementWithSharedPtr_Pooled);
BENCHMARK(BM_SmallObjectManagementWithAny);
BENCHMARK(BM_LargeObjectManagementWithProxy);
BENCHMARK(BM_LargeObjectManagementWithProxy_Pooled);
BENCHMARK(BM_LargeObjectManagementWithUniquePtr);
BENCHMARK(BM_LargeObjectManagementWithSharedPtr);
BENCHMARK(BM_LargeObjectManagementWithSharedPtr_Pooled);
BENCHMARK(BM_LargeObjectManagementWithAny);

}  // namespace
