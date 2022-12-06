project('CM4all Davos', ['c', 'cpp'], version: '0.20',
  meson_version: '>= 0.56',
  default_options: [
    'c_std=c11',
    'cpp_std=c++2a',
    'warning_level=3',
  ],
)

compiler = meson.get_compiler('cpp')
c_compiler = meson.get_compiler('c')

debug = get_option('b_ndebug') != 'true'

common_flags = [
  '-D_GNU_SOURCE',
]

test_common_flags = [
  '-Wcast-align',
  '-Wcast-qual',
  '-Wdouble-promotion',
  '-Wfloat-equal',
  '-Wmissing-declarations',
  '-Wmissing-format-attribute',
  '-Wmissing-noreturn',
  '-Wredundant-decls',
  '-Wshadow',
  '-Wundef',
  '-Wwrite-strings',
  '-Wvla',
  '-Wwrite-strings',

  # clang specific warning options:
  '-Wunreachable-code-aggressive',
  '-Wused-but-marked-unused',
]

test_global_common_flags = [
  '-fvisibility=hidden',
]

if not debug
  test_global_common_flags += [
    '-ffunction-sections',
    '-fdata-sections',
  ]

  add_global_link_arguments(
    compiler.get_supported_link_arguments(
      '-Wl,--gc-sections',
      '-Wl,--icf=all',
    ),
    language: 'cpp'
  )
endif

test_global_cxxflags = test_global_common_flags + [
]

test_global_cflags = test_global_common_flags + [
]

test_cxxflags = test_common_flags + [
  '-fno-threadsafe-statics',
  '-fmerge-all-constants',

  '-Wcomma-subscript',
  '-Wextra-semi',
  '-Wmismatched-tags',
  '-Woverloaded-virtual',
  '-Wsign-promo',
  '-Wvolatile',
  '-Wvirtual-inheritance',

  '-Wno-missing-field-initializers',

  # a vtable without a dtor is just fine
  '-Wno-non-virtual-dtor',

  # the only warnings we got from this are from formatted error
  # messages, and their truncation is harmless
  '-Wno-format-truncation',

  # clang specific warning options:
  '-Wcomma',
  '-Wheader-hygiene',
  '-Winconsistent-missing-destructor-override',
]

test_cflags = test_common_flags + [
  '-Waggregate-return',
  '-Wbad-function-cast',
  '-Wmissing-prototypes',
  '-Wnested-externs',
  '-Wstrict-prototypes',
]

add_global_arguments(common_flags, language: 'cpp')
add_global_arguments(common_flags, language: 'c')

add_global_arguments(compiler.get_supported_arguments(test_global_cxxflags), language: 'cpp')
add_global_arguments(c_compiler.get_supported_arguments(test_global_cflags), language: 'c')
add_project_arguments(compiler.get_supported_arguments(test_cxxflags), language: 'cpp')
add_project_arguments(c_compiler.get_supported_arguments(test_cflags), language: 'c')

expat = dependency('expat')

inc = include_directories('src', 'libcommon/src')

subdir('libcommon/src/util')
subdir('libcommon/src/uri')

util2 = static_library(
  'util2',
  'src/util/UriEscape.cxx',
  include_directories: inc,
)
util_dep = declare_dependency(
  link_with: util2,
  dependencies: [
    util_dep,
    uri_dep,
  ],
)

subdir('libcommon/src/time')
subdir('libcommon/src/http')
subdir('libcommon/src/io')
subdir('libcommon/src/system')
subdir('libcommon/src/was')

spawn = static_library('spawn',
  'libcommon/src/spawn/UserNamespace.cxx',
  include_directories: inc,
  dependencies: [
    system_dep,
    io_dep,
  ])
spawn_dep = declare_dependency(link_with: spawn)

executable(
  'davos-plain',
  'src/PivotRoot.cxx',
  'src/IsolatePath.cxx',
  'src/util.cxx',
  'src/mime_types.cxx',
  'src/uri_escape.cxx',
  'src/expat.cxx',
  'src/wxml.cxx',
  'src/error.cxx',
  'src/ETag.cxx',
  'src/IfMatch.cxx',
  'src/splice.cxx',
  'src/directory.cxx',
  'src/get.cxx',
  'src/put.cxx',
  'src/propfind.cxx',
  'src/proppatch.cxx',
  'src/lock.cxx',
  'src/other.cxx',
  'src/file.cxx',
  'src/main.cxx',
  include_directories: inc,
  dependencies: [
    expat,
    was_dep,
    http_dep,
    time_dep,
    util_dep,
    io_dep,
    system_dep,
    spawn_dep,
  ],
  install: true,
  install_dir: 'lib/cm4all/was/bin',
)

subdir('test')
subdir('doc')
