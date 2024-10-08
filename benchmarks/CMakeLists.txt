project(msft_proxy_benchmarks)

include(FetchContent)
# The policy uses the download time for timestamp, instead of the timestamp in the archive. This
# allows for proper rebuilds when a projects URL changes.
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
  benchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.9.0.tar.gz
  URL_HASH SHA256=35a77f46cc782b16fac8d3b107fbfbb37dcd645f7c28eee19f3b8e0758b48994
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable tests for google benchmark")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable google benchmark unit tests")
FetchContent_MakeAvailable(benchmark)

add_executable(msft_proxy_benchmarks
  proxy_invocation_benchmark_context.cpp
  proxy_invocation_benchmark.cpp
  proxy_management_benchmark.cpp
)
target_include_directories(msft_proxy_benchmarks PRIVATE .)
target_link_libraries(msft_proxy_benchmarks PRIVATE msft_proxy benchmark::benchmark benchmark::benchmark_main)

if (MSVC)
  target_compile_options(msft_proxy_benchmarks PRIVATE /W4)
else()
  target_compile_options(msft_proxy_benchmarks PRIVATE -Wall -Wextra -Wpedantic)
endif()
