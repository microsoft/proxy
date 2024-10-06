// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <benchmark/benchmark.h>

#include "proxy_invocation_benchmark_context.h"

void BM_SmallObjectInvocationViaProxy(benchmark::State& state) {
  for (auto _ : state) {
    for (auto& p : SmallObjectInvocationProxyTestData) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_SmallObjectInvocationViaVirtualFunction(benchmark::State& state) {
  for (auto _ : state) {
    for (auto& p : SmallObjectInvocationVirtualFunctionTestData) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaProxy(benchmark::State& state) {
  for (auto _ : state) {
    for (auto& p : LargeObjectInvocationProxyTestData) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaVirtualFunction(benchmark::State& state) {
  for (auto _ : state) {
    for (auto& p : LargeObjectInvocationVirtualFunctionTestData) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

BENCHMARK(BM_SmallObjectInvocationViaProxy);
BENCHMARK(BM_SmallObjectInvocationViaVirtualFunction);
BENCHMARK(BM_LargeObjectInvocationViaProxy);
BENCHMARK(BM_LargeObjectInvocationViaVirtualFunction);
