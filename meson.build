project('CM4all Davos', 'cpp', version: '0.24',
  meson_version: '>= 1.2',
  default_options: [
    'cpp_std=c++23',
    'warning_level=3',

    # This is only here to build subprojects as static libraries
    'default_library=static',

    'fmt:cpp_std=c++23',

    'libwas:apreq2=disabled',
    'libwas:xios=disabled',
    'libwas:documentation=disabled',
  ],
)

compiler = meson.get_compiler('cpp')

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

add_global_arguments(common_flags, language: 'cpp')

add_global_arguments(compiler.get_supported_arguments(test_global_cxxflags), language: 'cpp')
add_project_arguments(compiler.get_supported_arguments(test_cxxflags), language: 'cpp')

expat = dependency('expat')

inc = include_directories('src', 'libcommon/src')

subdir('libcommon/src/util')
subdir('libcommon/src/lib/fmt')
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
subdir('libcommon/src/io/linux')
subdir('libcommon/src/system')
subdir('libcommon/src/was')

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
    io_linux_dep,
    system_dep,
  ],
  install: true,
  install_dir: 'lib/cm4all/was/bin',
)

subdir('test')
subdir('doc')
