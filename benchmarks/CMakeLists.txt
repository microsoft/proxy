project(msft_proxy_benchmarks)

FetchContent_Declare(
  benchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.9.4.tar.gz
  URL_HASH SHA256=b334658edd35efcf06a99d9be21e4e93e092bd5f95074c1673d5c8705d95c104
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable tests for google benchmark")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable google benchmark unit tests")
FetchContent_MakeAvailable(benchmark)

add_executable(msft_proxy_benchmarks
  proxy_creation_benchmark.cpp
  proxy_operation_benchmark_context.cpp
  proxy_operation_benchmark.cpp
)
target_include_directories(msft_proxy_benchmarks PRIVATE .)
target_link_libraries(msft_proxy_benchmarks PRIVATE msft_proxy4::proxy benchmark::benchmark benchmark::benchmark_main)

if (MSVC)
  target_compile_options(msft_proxy_benchmarks PRIVATE /W4)
else()
  target_compile_options(msft_proxy_benchmarks PRIVATE -Wall -Wextra -Wpedantic $<$<CXX_COMPILER_ID:Clang>:-Wno-c++2b-extensions>)
endif()
