tests_modules_exe = executable(
  'msft_proxy_tests_modules',
  files('foo.cpp', 'impl.cpp', 'main.cpp'),
  implicit_include_directories: false,
  cpp_args: modules_interface_cflags,
  dependencies: [msft_proxy4_mod, gtest_dep],
  build_by_default: false,
)
test(
  'TestBasic',
  tests_modules_exe,
  suite: 'ProxyModuleSupportTests',
  protocol: 'gtest',
)
