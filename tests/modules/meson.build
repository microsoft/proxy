test(
  'ProxyModuleSupportTests',
  executable(
    'msft_proxy_tests_modules',
    files('foo.cpp', 'impl.cpp', 'main.cpp'),
    implicit_include_directories: false,
    cpp_args: modules_interface_cflags,
    dependencies: [msft_proxy4_mod, gtest_dep],
    build_by_default: false,
  ),
  protocol: 'gtest',
)
