tests_modules_exe = executable(
  'msft_proxy_tests_modules',
  files('foo.cpp', 'impl.cpp', 'main.cpp'),
  implicit_include_directories: false,
  cpp_args: modules_interface_cflags,
  dependencies: [msft_proxy4_mod, gtest_dep],
  build_by_default: false,
)

autogen_modules_tests = custom_target(
  command: [
    autogen_tests_exe,
    custom_target(
      command: [
        tests_modules_exe,
        '--gtest_list_tests',
        '--gtest_output=json:@OUTPUT@',
      ],
      output: 'tests.json',
    ),
    meson.current_source_dir() / 'meson.build',
  ],
  output: 'autogen.stamp',
  build_by_default: true,
)

#pragma autogen push
tests = {
  'ProxyModuleSupportTests': ['TestBasic'],
}
#pragma autogen pop

foreach suite : tests.keys()
  foreach name : tests[suite]
    test(
      name,
      tests_modules_exe,
      args: [f'--gtest_filter=@suite@.@name@'],
      protocol: 'gtest',
      suite: suite,
      depends: autogen_modules_tests,
    )
  endforeach
endforeach
