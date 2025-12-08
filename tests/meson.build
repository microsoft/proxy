test_srcs = [
  'proxy_creation_tests.cpp',
  'proxy_dispatch_tests.cpp',
  'proxy_fmt_format_tests.cpp',
  'proxy_format_tests.cpp',
  'proxy_integration_tests.cpp',
  'proxy_invocation_tests.cpp',
  'proxy_lifetime_tests.cpp',
  'proxy_reflection_tests.cpp',
  'proxy_regression_tests.cpp',
  'proxy_rtti_tests.cpp',
  'proxy_traits_tests.cpp',
  'proxy_view_tests.cpp',
]

foreach src : test_srcs
  test_deps = [msft_proxy4_dep, gtest_dep]
  name = src.substring(6, -10)
  if name.contains('fmt')
    test_deps += fmt_dep
  endif
  title = ''
  foreach part : name.split('_')
    title += part.substring(0, 1).to_upper() + part.substring(1)
  endforeach

  test(
    f'Proxy@title@Tests',
    executable(
      f'msft_proxy_@name@_tests',
      src,
      implicit_include_directories: false,
      dependencies: test_deps,
      build_by_default: false,
    ),
    protocol: 'gtest',
  )
endforeach

freestanding = get_option('tests_freestanding')
freestanding_cflags = ['-ffreestanding', '-fno-exceptions', '-fno-rtti']
freestanding_ldflags = ['-nodefaultlibs']

if cxx.has_multi_arguments(
  freestanding_cflags,
  freestanding_ldflags,
  required: freestanding,
)
  libc_dep = cxx.find_library(
    'c',
    required: freestanding,
  )

  if libc_dep.found()
    test(
      'ProxyFreestandingSupportTests',
      executable(
        'msft_proxy_freestanding_tests',
        files('freestanding/proxy_freestanding_tests.cpp'),
        implicit_include_directories: false,
        dependencies: [msft_proxy4_dep, libc_dep],
        cpp_args: freestanding_cflags + freestanding_ldflags,
        link_args: get_option('b_sanitize') == 'none' ? freestanding_ldflags : [],
        build_by_default: false,
      ),
    )
  endif
endif
