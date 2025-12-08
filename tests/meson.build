test_srcs = files(
  'proxy_creation_tests.cpp',
  'proxy_dispatch_tests.cpp',
  'proxy_format_tests.cpp',
  'proxy_integration_tests.cpp',
  'proxy_invocation_tests.cpp',
  'proxy_lifetime_tests.cpp',
  'proxy_reflection_tests.cpp',
  'proxy_regression_tests.cpp',
  'proxy_rtti_tests.cpp',
  'proxy_traits_tests.cpp',
  'proxy_view_tests.cpp',
)

test_deps = [msft_proxy4_dep, gtest_dep]

if fmt_dep.found()
  test_srcs += files(
    'proxy_fmt_format_tests.cpp',
  )
  test_deps += fmt_dep
endif

test(
  'ProxyTests',
  executable(
    'msft_proxy_tests',
    test_srcs,
    implicit_include_directories: false,
    dependencies: test_deps,
  ),
  protocol: 'gtest',
)

freestanding_cflags = ['-ffreestanding', '-fno-exceptions', '-fno-rtti']
freestanding_ldflags = ['-nodefaultlibs']

if cxx.has_multi_arguments(
  freestanding_cflags,
  freestanding_ldflags,
  required: false,
)
  libc_dep = cxx.find_library(
    'c',
    required: false,
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
      ),
    )
  endif
endif
