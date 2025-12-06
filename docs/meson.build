extract_doctest_exe = find_program('../tools/meson_extract_doctest.py')

docs = run_command(
  extract_doctest_exe,
  meson.current_source_dir(),
  capture: true,
  check: true,
).stdout().strip('\0').split('\0')

examples = []

foreach doc : docs
  deps = [msft_proxy4_dep]
  if doc.contains('fmt')
    deps += fmt_dep
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
  )
endforeach

alias_target('examples', examples)
