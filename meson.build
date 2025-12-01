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
modules = get_option('modules')
modules_interface_spec = {
  'gcc': {
    'flags': [
      '-fmodules',
      '-fdeps-format=p1689r5',
      '-DWINPTHREAD_COND_DECL=WINPTHREADS_ALWAYS_INLINE',
    ],
    'check_only': ['-M'],
  },
  'msvc': {
    'flags': ['/interface'],
  },
}[cxx.get_argument_syntax()]
if cxx.has_multi_arguments(
  modules_interface_spec.get('flags'),
  modules_interface_spec.get('check_only', []),
  required: modules,
)
  modules_interface_cflags = modules_interface_spec['flags']
  msft_proxy4_mod = declare_dependency(
    include_directories: inc,
    dependencies: msft_proxy4_dep,
    link_with: static_library(
      'msft_proxy4_module',
      hdrs_mod,
      include_directories: inc,
      cpp_args: modules_interface_cflags,
      implicit_include_directories: false,
      build_by_default: true,
    ),
  )
else
  modules_interface_cflags = []
  msft_proxy4_mod = disabler()
endif

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
meson.override_dependency(meson.project_name() + '_module', msft_proxy4_mod)

tests = get_option('tests')
gtest_dep = dependency(
  'gtest_main',
  main: true,
  required: tests,
)
fmt_spec = ['>=6.1.0']
fmt_dep = dependency(
  'fmt',
  version: fmt_spec,
  required: tests,
)
subdir(
  'tests',
  if_found: [gtest_dep, fmt_dep],
)

benchmarks = get_option('benchmarks')
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
