// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <benchmark/benchmark.h>

#include "proxy_small_object_invocation_benchmark_context.h"

void BM_SmallObjectInvocationViaProxy(benchmark::State& state) {
  for (auto _ : state) {
    for (auto& p : ProxyTestData) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_SmallObjectInvocationViaVirtualFunction(benchmark::State& state) {
  for (auto _ : state) {
    for (auto& p : VirtualFunctionTestData) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

BENCHMARK(BM_SmallObjectInvocationViaProxy);
BENCHMARK(BM_SmallObjectInvocationViaVirtualFunction);
