// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <any>
#include <array>
#include <forward_list>

#include <benchmark/benchmark.h>

#include "proxy.h"

namespace {

constexpr int TestManagedObjectCount = 12000;

using TestSmallObjectType1 = int;
using TestSmallObjectType2 = std::shared_ptr<int>;
using TestSmallObjectType3 = std::forward_list<double>;

}  // namespace

struct AnyCopyable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>  // Although it's not used, std::any supports copy
    ::build {};

void BM_SmallObjectManagementWithProxy(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<pro::proxy<AnyCopyable>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += 3) {
      data.push_back(pro::make_proxy<AnyCopyable>(123));
      data.push_back(pro::make_proxy<AnyCopyable, std::shared_ptr<int>>());
      data.push_back(pro::make_proxy<AnyCopyable, std::forward_list<double>>());
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_SmallObjectManagementWithAny(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::any> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += 3) {
      data.emplace_back(123);
      data.emplace_back(std::shared_ptr<int>{});
      data.emplace_back(std::forward_list<double>{});
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithProxy(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<pro::proxy<AnyCopyable>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; ++i) {
      data.push_back(pro::make_proxy<AnyCopyable, std::array<int, 32>>());
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithAny(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::any> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; ++i) {
      data.emplace_back(std::array<int, 32>{});
    }
    benchmark::DoNotOptimize(data);
  }
}

BENCHMARK(BM_SmallObjectManagementWithProxy);
BENCHMARK(BM_SmallObjectManagementWithAny);
BENCHMARK(BM_LargeObjectManagementWithProxy);
BENCHMARK(BM_LargeObjectManagementWithAny);
