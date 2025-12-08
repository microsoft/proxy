project(
  'msft_proxy4',
  'cpp',
  version: '4.0.1',
  license: 'MIT',
  license_files: 'LICENSE',
  meson_version: '>=1.3',
  default_options: {
    'cpp_std': ['vc++latest', 'c++26', 'c++23', 'c++20'],
    'warning_level': '3',
  },
)

cxx = meson.get_compiler('cpp')
if cxx.compute_int('__has_cpp_attribute(msvc::no_unique_address)') != 0 and not cxx.compiles(
  '''
  struct A {};

  struct B {
    [[msvc::no_unique_address]] A val;
  };

  struct C : B {
    char x;
  };

  static_assert(sizeof(C) == 1);
  ''',
  name: 'msvc::no_unique_address behaves correctly',
)
  error(
    'unsupported compiler:',
    'https://github.com/llvm/llvm-project/issues/143245',
  )
endif
if cxx.get_argument_syntax() == 'msvc'
  add_project_arguments(
    '/bigobj',
    language: 'cpp',
  )
endif

inc = include_directories('include')

hdrs = files(
  'include/proxy/proxy.h',
  'include/proxy/proxy_fmt.h',
  'include/proxy/proxy_macros.h',
)
hdrs_v4 = files(
  'include/proxy/v4/proxy.h',
  'include/proxy/v4/proxy_fmt.h',
  'include/proxy/v4/proxy_macros.h',
)
hdrs_mod = files('include/proxy/v4/proxy.ixx')

msft_proxy4_dep = declare_dependency(
  sources: hdrs + hdrs_v4,
  include_directories: inc,
)

install_headers(
  hdrs,
  subdir: 'proxy',
)
install_headers(
  hdrs_v4,
  subdir: 'proxy/v4',
)
install_headers(
  hdrs_mod,
  subdir: 'proxy/v4',
)
pkgconfig = import(
  'pkgconfig',
  required: false,
)
if pkgconfig.found()
  pkgconfig.generate(
    name: meson.project_name(),
    description: 'Next Generation Polymorphism in C++',
    url: 'https://microsoft.github.io/proxy/',
  )
endif

meson.override_dependency(meson.project_name(), msft_proxy4_dep)

tests = get_option('tests').disable_auto_if(meson.is_subproject())
gtest_dep = dependency(
  'gtest_main',
  main: true,
  required: tests,
)
fmt_dep = dependency(
  'fmt',
  version: '>=6.1.0',
  required: false,
  disabler: true,
)
subdir(
  'tests',
  if_found: gtest_dep,
)

benchmarks = get_option('benchmarks').disable_auto_if(meson.is_subproject())
benchmark_dep = dependency(
  'benchmark_main',
  required: benchmarks,
)
subdir(
  'benchmarks',
  if_found: benchmark_dep,
)

examples = get_option('examples').disable_auto_if(meson.is_subproject())
if examples.allowed()
  subdir('docs')
endif
