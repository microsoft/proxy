maybe_fmt_dep = dependency(
  'fmt',
  version: fmt_spec,
  required: false,
  disabler: true,
)

autogen_glob = custom_target(
  command: [autogen_doctest_exe, meson.current_source_dir()],
  output: 'autogen.stamp',
  build_by_default: true,
)

#pragma autogen push
docs = [
  'spec/PRO_DEF_FREE_AS_MEM_DISPATCH.md',
  'spec/PRO_DEF_FREE_DISPATCH.md',
  'spec/PRO_DEF_MEM_DISPATCH.md',
  'spec/allocate_proxy.md',
  'spec/allocate_proxy_shared.md',
  'spec/facade_aware_overload_t.md',
  'spec/inplace_proxiable_target.md',
  'spec/is_bitwise_trivially_relocatable.md',
  'spec/make_proxy.md',
  'spec/make_proxy_inplace.md',
  'spec/make_proxy_shared.md',
  'spec/make_proxy_view.md',
  'spec/msft_lib_proxy.md',
  'spec/proxiable.md',
  'spec/proxiable_target.md',
  'spec/proxy_invoke.md',
  'spec/proxy_reflect.md',
  'spec/proxy_view.md',
  'spec/skills_as_view.md',
  'spec/skills_as_weak.md',
  'spec/skills_fmt_format.md',
  'spec/skills_format.md',
  'spec/skills_slim.md',
  'spec/weak_proxy.md',
  'spec/basic_facade_builder/README.md',
  'spec/basic_facade_builder/add_convention.md',
  'spec/basic_facade_builder/add_facade.md',
  'spec/basic_facade_builder/add_reflection.md',
  'spec/basic_facade_builder/add_skill.md',
  'spec/basic_facade_builder/build.md',
  'spec/basic_facade_builder/restrict_layout.md',
  'spec/basic_facade_builder/support_copy.md',
  'spec/basic_facade_builder/support_destruction.md',
  'spec/basic_facade_builder/support_relocation.md',
  'spec/explicit_conversion_dispatch/README.md',
  'spec/implicit_conversion_dispatch/README.md',
  'spec/operator_dispatch/README.md',
  'spec/proxy/README.md',
  'spec/proxy/constructor.md',
  'spec/proxy/destructor.md',
  'spec/proxy/emplace.md',
  'spec/proxy/friend_operator_equality.md',
  'spec/proxy/friend_swap.md',
  'spec/proxy/indirection.md',
  'spec/proxy/operator_bool.md',
  'spec/proxy/reset.md',
  'spec/skills_rtti/README.md',
  'spec/skills_rtti/proxy_cast.md',
  'spec/skills_rtti/proxy_typeid.md',
  'spec/substitution_dispatch/README.md',
  'spec/weak_dispatch/README.md',
]
#pragma autogen pop

examples = []

foreach doc : docs
  deps = [msft_proxy4_dep]
  if doc.contains('fmt')
    deps += maybe_fmt_dep
  endif

  name = doc.replace('/', '_').substring(0, -3)
  example_exe = executable(
    f'example_@name@',
    custom_target(
      command: [extract_doctest_exe, '@INPUT@', '@OUTPUT@'],
      input: doc,
      output: f'@name@.cpp',
    ),
    extra_files: doc,
    implicit_include_directories: false,
    dependencies: deps,
    build_by_default: false,
  )
  examples += example_exe
  test(
    name,
    example_exe,
    suite: 'ProxyExamples',
    depends: autogen_glob,
  )
endforeach

alias_target('examples', examples)
