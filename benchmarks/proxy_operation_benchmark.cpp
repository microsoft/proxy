// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <benchmark/benchmark.h>

#include "proxy_operation_benchmark_context.h"

namespace {

void BM_SmallObjectInvocationViaProxy(benchmark::State& state) {
  auto data = GenerateSmallObjectProxyTestData();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_SmallObjectInvocationViaProxy_Shared(benchmark::State& state) {
  auto data = GenerateSmallObjectProxyTestData_Shared();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_SmallObjectInvocationViaProxyView(benchmark::State& state) {
  auto data = GenerateSmallObjectProxyTestData();
  std::vector<pro::proxy_view<InvocationTestFacade>> views(data.begin(),
                                                           data.end());
  for (auto _ : state) {
    for (auto& p : views) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_SmallObjectInvocationViaVirtualFunction(benchmark::State& state) {
  auto data = GenerateSmallObjectVirtualFunctionTestData();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_SmallObjectInvocationViaVirtualFunction_Shared(
    benchmark::State& state) {
  auto data = GenerateSmallObjectVirtualFunctionTestData_Shared();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_SmallObjectInvocationViaVirtualFunction_RawPtr(
    benchmark::State& state) {
  auto data = GenerateSmallObjectVirtualFunctionTestData();
  std::vector<InvocationTestBase*> ptrs;
  ptrs.reserve(data.size());
  for (auto& p : data) {
    ptrs.push_back(p.get());
  }
  for (auto _ : state) {
    for (auto& p : ptrs) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaProxy(benchmark::State& state) {
  auto data = GenerateLargeObjectProxyTestData();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaProxy_Shared(benchmark::State& state) {
  auto data = GenerateLargeObjectProxyTestData_Shared();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaProxyView(benchmark::State& state) {
  auto data = GenerateLargeObjectProxyTestData();
  std::vector<pro::proxy_view<InvocationTestFacade>> views(data.begin(),
                                                           data.end());
  for (auto _ : state) {
    for (auto& p : views) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaVirtualFunction(benchmark::State& state) {
  auto data = GenerateLargeObjectVirtualFunctionTestData();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaVirtualFunction_Shared(
    benchmark::State& state) {
  auto data = GenerateLargeObjectVirtualFunctionTestData();
  for (auto _ : state) {
    for (auto& p : data) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_LargeObjectInvocationViaVirtualFunction_RawPtr(
    benchmark::State& state) {
  auto data = GenerateLargeObjectVirtualFunctionTestData();
  std::vector<InvocationTestBase*> ptrs;
  ptrs.reserve(data.size());
  for (auto& p : data) {
    ptrs.push_back(p.get());
  }
  for (auto _ : state) {
    for (auto& p : ptrs) {
      int result = p->Fun();
      benchmark::DoNotOptimize(result);
    }
  }
}

void BM_SmallObjectRelocationViaProxy(benchmark::State& state) {
  auto data = GenerateSmallObjectProxyTestData();
  decltype(data) buffer(data.size());
  for (auto _ : state) {
    for (std::size_t i = 0; i < data.size(); ++i) {
      buffer[i] = std::move(data[i]);
    }
    benchmark::DoNotOptimize(buffer);
    std::swap(data, buffer);
  }
}

void BM_SmallObjectRelocationViaProxy_NothrowRelocatable(
    benchmark::State& state) {
  auto data = GenerateSmallObjectProxyTestData_NothrowRelocatable();
  decltype(data) buffer(data.size());
  for (auto _ : state) {
    for (std::size_t i = 0; i < data.size(); ++i) {
      buffer[i] = std::move(data[i]);
    }
    benchmark::DoNotOptimize(buffer);
    std::swap(data, buffer);
  }
}

void BM_SmallObjectRelocationViaUniquePtr(benchmark::State& state) {
  auto data = GenerateSmallObjectVirtualFunctionTestData();
  decltype(data) buffer(data.size());
  for (auto _ : state) {
    for (std::size_t i = 0; i < data.size(); ++i) {
      buffer[i] = std::move(data[i]);
    }
    benchmark::DoNotOptimize(buffer);
    std::swap(data, buffer);
  }
}

void BM_SmallObjectRelocationViaAny(benchmark::State& state) {
  auto data = GenerateSmallObjectAnyTestData();
  decltype(data) buffer(data.size());
  for (auto _ : state) {
    for (std::size_t i = 0; i < data.size(); ++i) {
      buffer[i] = std::move(data[i]);
    }
    benchmark::DoNotOptimize(buffer);
    std::swap(data, buffer);
  }
}

void BM_LargeObjectRelocationViaProxy(benchmark::State& state) {
  auto data = GenerateLargeObjectProxyTestData();
  decltype(data) buffer(data.size());
  for (auto _ : state) {
    for (std::size_t i = 0; i < data.size(); ++i) {
      buffer[i] = std::move(data[i]);
    }
    benchmark::DoNotOptimize(buffer);
    std::swap(data, buffer);
  }
}

void BM_LargeObjectRelocationViaProxy_NothrowRelocatable(
    benchmark::State& state) {
  auto data = GenerateLargeObjectProxyTestData_NothrowRelocatable();
  decltype(data) buffer(data.size());
  for (auto _ : state) {
    for (std::size_t i = 0; i < data.size(); ++i) {
      buffer[i] = std::move(data[i]);
    }
    benchmark::DoNotOptimize(buffer);
    std::swap(data, buffer);
  }
}

void BM_LargeObjectRelocationViaUniquePtr(benchmark::State& state) {
  auto data = GenerateLargeObjectVirtualFunctionTestData();
  decltype(data) buffer(data.size());
  for (auto _ : state) {
    for (std::size_t i = 0; i < data.size(); ++i) {
      buffer[i] = std::move(data[i]);
    }
    benchmark::DoNotOptimize(buffer);
    std::swap(data, buffer);
  }
}

void BM_LargeObjectRelocationViaAny(benchmark::State& state) {
  auto data = GenerateLargeObjectAnyTestData();
  decltype(data) buffer(data.size());
  for (auto _ : state) {
    for (std::size_t i = 0; i < data.size(); ++i) {
      buffer[i] = std::move(data[i]);
    }
    benchmark::DoNotOptimize(buffer);
    std::swap(data, buffer);
  }
}

BENCHMARK(BM_SmallObjectInvocationViaProxy);
BENCHMARK(BM_SmallObjectInvocationViaProxy_Shared);
BENCHMARK(BM_SmallObjectInvocationViaProxyView);
BENCHMARK(BM_SmallObjectInvocationViaVirtualFunction);
BENCHMARK(BM_SmallObjectInvocationViaVirtualFunction_Shared);
BENCHMARK(BM_SmallObjectInvocationViaVirtualFunction_RawPtr);
BENCHMARK(BM_LargeObjectInvocationViaProxy);
BENCHMARK(BM_LargeObjectInvocationViaProxy_Shared);
BENCHMARK(BM_LargeObjectInvocationViaProxyView);
BENCHMARK(BM_LargeObjectInvocationViaVirtualFunction);
BENCHMARK(BM_LargeObjectInvocationViaVirtualFunction_Shared);
BENCHMARK(BM_LargeObjectInvocationViaVirtualFunction_RawPtr);
BENCHMARK(BM_SmallObjectRelocationViaProxy);
BENCHMARK(BM_SmallObjectRelocationViaProxy_NothrowRelocatable);
BENCHMARK(BM_SmallObjectRelocationViaUniquePtr);
BENCHMARK(BM_SmallObjectRelocationViaAny);
BENCHMARK(BM_LargeObjectRelocationViaProxy);
BENCHMARK(BM_LargeObjectRelocationViaProxy_NothrowRelocatable);
BENCHMARK(BM_LargeObjectRelocationViaUniquePtr);
BENCHMARK(BM_LargeObjectRelocationViaAny);

} // namespace
