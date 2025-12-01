fs = import('fs')

maybe_fmt_dep = dependency(
  'fmt',
  version: fmt_spec,
  required: false,
  disabler: true,
)

# glob **.md
docs = [
  'README.md',
  'faq.md',
  'modules_support.md',
  'spec/PRO_DEF_FREE_AS_MEM_DISPATCH.md',
  'spec/PRO_DEF_FREE_DISPATCH.md',
  'spec/PRO_DEF_MEM_DISPATCH.md',
  'spec/ProAccessible.md',
  'spec/ProBasicConvention.md',
  'spec/ProBasicFacade.md',
  'spec/ProBasicReflection.md',
  'spec/ProConvention.md',
  'spec/ProDispatch.md',
  'spec/ProFacade.md',
  'spec/ProOverload.md',
  'spec/ProReflection.md',
  'spec/README.md',
  'spec/allocate_proxy.md',
  'spec/allocate_proxy_shared.md',
  'spec/bad_proxy_cast.md',
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
  'spec/constraint_level.md',
  'spec/explicit_conversion_dispatch/README.md',
  'spec/explicit_conversion_dispatch/accessor.md',
  'spec/explicit_conversion_dispatch/operator_call.md',
  'spec/facade.md',
  'spec/facade_aware_overload_t.md',
  'spec/implicit_conversion_dispatch/README.md',
  'spec/implicit_conversion_dispatch/accessor.md',
  'spec/implicit_conversion_dispatch/operator_call.md',
  'spec/inplace_proxiable_target.md',
  'spec/is_bitwise_trivially_relocatable.md',
  'spec/make_proxy.md',
  'spec/make_proxy_inplace.md',
  'spec/make_proxy_shared.md',
  'spec/make_proxy_view.md',
  'spec/msft_lib_proxy.md',
  'spec/not_implemented.md',
  'spec/operator_dispatch/README.md',
  'spec/operator_dispatch/accessor.md',
  'spec/operator_dispatch/operator_call.md',
  'spec/proxiable.md',
  'spec/proxiable_target.md',
  'spec/proxy/README.md',
  'spec/proxy/assignment.md',
  'spec/proxy/constructor.md',
  'spec/proxy/destructor.md',
  'spec/proxy/emplace.md',
  'spec/proxy/friend_operator_equality.md',
  'spec/proxy/friend_swap.md',
  'spec/proxy/indirection.md',
  'spec/proxy/operator_bool.md',
  'spec/proxy/reset.md',
  'spec/proxy/swap.md',
  'spec/proxy_indirect_accessor.md',
  'spec/proxy_invoke.md',
  'spec/proxy_reflect.md',
  'spec/proxy_view.md',
  'spec/skills_as_view.md',
  'spec/skills_as_weak.md',
  'spec/skills_fmt_format.md',
  'spec/skills_format.md',
  'spec/skills_rtti/README.md',
  'spec/skills_rtti/proxy_cast.md',
  'spec/skills_rtti/proxy_typeid.md',
  'spec/skills_slim.md',
  'spec/substitution_dispatch/README.md',
  'spec/substitution_dispatch/accessor.md',
  'spec/substitution_dispatch/operator_call.md',
  'spec/weak_dispatch/README.md',
  'spec/weak_dispatch/operator_call.md',
  'spec/weak_proxy.md',
]

docdir = get_option('docdir')
if docdir == ''
  docdir = get_option('datadir') / 'doc' / meson.project_name()
endif
install_data(
  docs,
  follow_symlinks: true,
  install_dir: docdir,
  preserve_path: true,
  install_tag: 'docs',
)

examples = []

foreach doc : docs
  code = ''
  deps = [msft_proxy4_dep]

  state = 0
  lines = fs.read(doc).splitlines()
  foreach line : lines
    if state == 0
      if line.contains('## Example')
        if code != ''
          error(
            'File',
            doc,
            'contains more than one "## Example" C++ code block.',
          )
        endif
        state = 1
      endif
    elif state == 1
      if line == ''
        state = 2
      else
        state = 0
      endif
    elif state == 2
      if line == '```cpp'
        state = 3
      else
        state = 0
      endif
    elif state == 3
      if line != '```'
        if line.contains('<proxy/proxy_fmt.h>')
          deps += maybe_fmt_dep
        endif
        code += line + '\n'
      else
        code = code.substring(0, -1)
        state = 0
      endif
    endif
  endforeach
  if code != ''
    cfg = {
      'MD_PATH': 'docs' / doc,
      'CODE': code,
    }
    name = doc.replace('/', '_').substring(0, -3)
    example_exe = executable(
      f'example_@name@',
      configure_file(
        configuration: cfg,
        input: 'example.cpp.in',
        output: f'example_@name@.cpp',
      ),
      implicit_include_directories: false,
      dependencies: deps,
      build_by_default: false,
    )
    examples += example_exe
    test(
      name,
      example_exe,
      suite: 'ProxyExamples',
    )
  endif
endforeach

alias_target('examples', examples)
