// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <any>
#include <array>
#include <deque>
#include <forward_list>
#include <string>
#include <unordered_map>

#include <benchmark/benchmark.h>

#include "proxy.h"

namespace {

constexpr int TestManagedObjectCount = 12000;

using TestSmallObjectType1 = int;
using TestSmallObjectType2 = std::shared_ptr<int>;
using TestSmallObjectType3 = std::forward_list<double>;

using TestLargeObjectType1 = std::array<int, 32>;
struct TestLargeObjectType2 {
  int Field1;
  std::vector<int> Field2;
};
struct TestLargeObjectType3 {
  std::unordered_map<int, int> Field1;
  std::deque<int> Field2;
  std::string Field3;
};

}  // namespace

struct AnyCopyable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>  // Although it's not used, std::any supports copy
    ::build {};

void BM_SmallObjectManagementWithProxy(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<pro::proxy<AnyCopyable>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += 3) {
      data.push_back(pro::make_proxy<AnyCopyable, TestSmallObjectType1>());
      data.push_back(pro::make_proxy<AnyCopyable, TestSmallObjectType2>());
      data.push_back(pro::make_proxy<AnyCopyable, TestSmallObjectType3>());
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_SmallObjectManagementWithAny(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::any> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += 3) {
      data.emplace_back(TestSmallObjectType1{});
      data.emplace_back(TestSmallObjectType2{});
      data.emplace_back(TestSmallObjectType3{});
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithProxy(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<pro::proxy<AnyCopyable>> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += 3) {
      data.push_back(pro::make_proxy<AnyCopyable, TestLargeObjectType1>());
      data.push_back(pro::make_proxy<AnyCopyable, TestLargeObjectType2>());
      data.push_back(pro::make_proxy<AnyCopyable, TestLargeObjectType3>());
    }
    benchmark::DoNotOptimize(data);
  }
}

void BM_LargeObjectManagementWithAny(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<std::any> data;
    data.reserve(TestManagedObjectCount);
    for (int i = 0; i < TestManagedObjectCount; i += 3) {
      data.emplace_back(TestLargeObjectType1{});
      data.emplace_back(TestLargeObjectType2{});
      data.emplace_back(TestLargeObjectType3{});
    }
    benchmark::DoNotOptimize(data);
  }
}

BENCHMARK(BM_SmallObjectManagementWithProxy);
BENCHMARK(BM_SmallObjectManagementWithAny);
BENCHMARK(BM_LargeObjectManagementWithProxy);
BENCHMARK(BM_LargeObjectManagementWithAny);
