benchmark(
  'ProxyBenchmarks',
  executable(
    'msft_proxy_benchmarks',
    files(
      'proxy_creation_benchmark.cpp',
      'proxy_operation_benchmark.cpp',
      'proxy_operation_benchmark_context.cpp',
    ),
    implicit_include_directories: false,
    dependencies: [msft_proxy4_dep, benchmark_dep],
  ),
)
