benchmarks_exe = executable(
  'msft_proxy_benchmarks',
  files(
    'proxy_creation_benchmark.cpp',
    'proxy_operation_benchmark.cpp',
    'proxy_operation_benchmark_context.cpp',
  ),
  implicit_include_directories: false,
  dependencies: [msft_proxy4_dep, benchmark_dep],
  build_by_default: false,
)

autogen_benchmarks = custom_target(
  command: [
    autogen_benchmarks_exe,
    custom_target(
      command: [benchmarks_exe, '--benchmark_list_tests'],
      capture: true,
      output: 'tests.txt',
    ),
    meson.current_source_dir() / 'meson.build',
  ],
  output: 'autogen.stamp',
  build_by_default: true,
)

#pragma autogen push
benchmarks = {
  'ObjectCreation': [
    'BM_SmallObjectCreationWithProxy_Shared',
    'BM_SmallObjectCreationWithProxy_SharedPooled',
    'BM_SmallObjectCreationWithUniquePtr',
    'BM_SmallObjectCreationWithSharedPtr',
    'BM_SmallObjectCreationWithSharedPtr_Pooled',
    'BM_SmallObjectCreationWithAny',
    'BM_LargeObjectCreationWithProxy',
    'BM_LargeObjectCreationWithProxy_Pooled',
    'BM_LargeObjectCreationWithProxy_Shared',
    'BM_LargeObjectCreationWithProxy_SharedPooled',
    'BM_LargeObjectCreationWithUniquePtr',
    'BM_LargeObjectCreationWithSharedPtr',
    'BM_LargeObjectCreationWithSharedPtr_Pooled',
    'BM_LargeObjectCreationWithAny',
  ],
  'ObjectInvocation': [
    'BM_SmallObjectInvocationViaProxy_Shared',
    'BM_SmallObjectInvocationViaProxyView',
    'BM_SmallObjectInvocationViaVirtualFunction',
    'BM_SmallObjectInvocationViaVirtualFunction_Shared',
    'BM_SmallObjectInvocationViaVirtualFunction_RawPtr',
    'BM_LargeObjectInvocationViaProxy',
    'BM_LargeObjectInvocationViaProxy_Shared',
    'BM_LargeObjectInvocationViaProxyView',
    'BM_LargeObjectInvocationViaVirtualFunction',
    'BM_LargeObjectInvocationViaVirtualFunction_Shared',
    'BM_LargeObjectInvocationViaVirtualFunction_RawPtr',
  ],
  'ObjectRelocation': [
    'BM_SmallObjectRelocationViaProxy_NothrowRelocatable',
    'BM_SmallObjectRelocationViaUniquePtr',
    'BM_SmallObjectRelocationViaAny',
    'BM_LargeObjectRelocationViaProxy',
    'BM_LargeObjectRelocationViaProxy_NothrowRelocatable',
    'BM_LargeObjectRelocationViaUniquePtr',
    'BM_LargeObjectRelocationViaAny',
  ],
}
#pragma autogen pop

foreach suite : benchmarks.keys()
  foreach name : benchmarks[suite]
    benchmark(
      name,
      benchmarks_exe,
      args: [f'--benchmark_filter=^@name@$'],
      suite: suite,
      depends: autogen_benchmarks,
    )
  endforeach
endforeach
