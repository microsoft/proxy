// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <any>
#include <array>
#include <deque>
#include <forward_list>
#include <unordered_map>

#include <benchmark/benchmark.h>

#include "proxy.h"

namespace {

constexpr int TestManagedObjectCount = 12000;

}  // namespace

struct AnyCopyable : pro::facade_builder::support_copy<pro::constraint_level::nontrivial>::build {};

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
    for (int i = 0; i < TestManagedObjectCount; i += 3) {
      data.push_back(pro::make_proxy<AnyCopyable, std::array<int, 16>>());
      data.push_back(pro::make_proxy<AnyCopyable, std::deque<double>>());
      data.push_back(pro::make_proxy<AnyCopyable, std::unordered_map<int, int>>());
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithAny(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::any> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += 3) {
      data.emplace_back(std::array<int, 16>{});
      data.emplace_back(std::deque<double>{});
      data.emplace_back(std::unordered_map<int, int>{});
    }
    benchmark::DoNotOptimize(data);
  }
}

BENCHMARK(BM_SmallObjectManagementWithProxy);
BENCHMARK(BM_SmallObjectManagementWithAny);
BENCHMARK(BM_LargeObjectManagementWithProxy);
BENCHMARK(BM_LargeObjectManagementWithAny);
