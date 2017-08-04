gtest_compile_args = [
  '-Wno-undef',
]

if compiler.get_id() == 'gcc'
  gtest_compile_args += [
    '-Wno-suggest-attribute=format',
    '-Wno-suggest-attribute=noreturn',
    '-Wno-missing-declarations',

    # needed on Jessie for gtest's IsNullLiteralHelper
    '-Wno-conversion-null',
  ]
endif

gtest = declare_dependency(
  dependencies: [dependency('gtest', main: true)],
  compile_args: gtest_compile_args,
)

test('t_uri_escape', executable('t_uri_escape',
  't_uri_escape.cxx',
  include_directories: inc,
  install: false,
  dependencies: [
    gtest,
    libinline,
    util_dep,
  ]))
