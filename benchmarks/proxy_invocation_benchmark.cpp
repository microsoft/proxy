// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <benchmark/benchmark.h>

#include "proxy_invocation_benchmark_context.h"

namespace {

void BM_SmallObjectInvocationViaProxy(benchmark::State& state) {
  auto data = GenerateSmallObjectInvocationProxyTestData();
  std::vector<pro::proxy_view<const InvocationTestFacade>> views{data.begin(), data.end()};
  for (auto _ : state) {
    for (auto& p : views) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_SmallObjectInvocationViaProxyView(benchmark::State& state) {
  auto data = GenerateSmallObjectInvocationProxyTestData();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_SmallObjectInvocationViaVirtualFunction(benchmark::State& state) {
  auto data = GenerateSmallObjectInvocationVirtualFunctionTestData();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaProxy(benchmark::State& state) {
  auto data = GenerateLargeObjectInvocationProxyTestData();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaProxyView(benchmark::State& state) {
  auto data = GenerateLargeObjectInvocationProxyTestData();
  std::vector<pro::proxy_view<const InvocationTestFacade>> views{data.begin(), data.end()};
  for (auto _ : state) {
    for (auto& p : views) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaVirtualFunction(benchmark::State& state) {
  auto data = GenerateLargeObjectInvocationVirtualFunctionTestData();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

BENCHMARK(BM_SmallObjectInvocationViaProxy);
BENCHMARK(BM_SmallObjectInvocationViaProxyView);
BENCHMARK(BM_SmallObjectInvocationViaVirtualFunction);
BENCHMARK(BM_LargeObjectInvocationViaProxy);
BENCHMARK(BM_LargeObjectInvocationViaProxyView);
BENCHMARK(BM_LargeObjectInvocationViaVirtualFunction);

}  // namespace
