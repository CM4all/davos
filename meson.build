project('CM4all Davos', ['c', 'cpp'], version: '0.11',
  default_options: [
    'c_std=c99',
    'cpp_std=c++14'
  ],
)

compiler = meson.get_compiler('cpp')

common_flags = [
  '-D_REENTRANT', '-D_GNU_SOURCE',
]

add_global_arguments(common_flags,
  '-Wall',
  '-Wextra',
  '-Wmissing-prototypes', '-Wstrict-prototypes',
  '-Wwrite-strings', '-Wcast-qual', '-Wcast-align', '-Wfloat-equal',
  '-Wbad-function-cast',
  '-Wshadow', '-Wpointer-arith', '-Wsign-compare',
  '-Waggregate-return',
  '-Wmissing-declarations', '-Wmissing-noreturn', '-Wmissing-format-attribute',
  '-Wredundant-decls', '-Wno-long-long', '-Wundef',
  '-Wnested-externs',
  '-Wunused',
  '-Wundef',
  '-pedantic',
  language: 'c')

add_global_arguments(common_flags,
  '-Wall',
  '-Wextra',
  '-Wwrite-strings', '-Wcast-qual', '-Wcast-align', '-Wfloat-equal',
  '-Wshadow', '-Wpointer-arith', '-Wsign-compare',
  '-Wmissing-declarations', '-Wmissing-noreturn', '-Wmissing-format-attribute',
  '-Wredundant-decls', '-Wno-long-long', '-Wundef',
  '-Wno-missing-field-initializers',
  '-Wno-non-virtual-dtor',
  language: 'cpp')

expat = dependency('expat')

libwas = dependency('libcm4all-was-simple', version: '>= 1.9')
libfox = dependency('libcm4all-fox', version: '>= 2.0')
libod = dependency('libcm4all-od-setup', version: '>= 1.2')

inc = include_directories('src', 'libcommon/src')

util = static_library('util',
  'libcommon/src/util/HexFormat.c',
  'libcommon/src/util/PrintException.cxx',
  'libcommon/src/util/StringUtil.cxx',
  'libcommon/src/util/StringView.cxx',
  'src/util/UriEscape.cxx',
  include_directories: inc,
  dependencies: [
  ])
util_dep = declare_dependency(link_with: util)

time = static_library('time',
  'libcommon/src/time/gmtime.c',
  include_directories: inc,
  dependencies: [
  ])
time_dep = declare_dependency(link_with: time)

http = static_library('http',
  'libcommon/src/http/List.cxx',
  'libcommon/src/http/Date.cxx',
  'libcommon/src/http/Range.cxx',
  include_directories: inc,
  dependencies: [
    util_dep,
  ])
http_dep = declare_dependency(link_with: http)

system = static_library('system',
  'libcommon/src/system/BindMount.cxx',
  include_directories: inc,
  dependencies: [
  ])
system_dep = declare_dependency(link_with: system)

io = static_library('io',
  'libcommon/src/io/FileDescriptor.cxx',
  'libcommon/src/io/FileWriter.cxx',
  include_directories: inc,
  dependencies: [
  ])
io_dep = declare_dependency(link_with: io)

executable('davos-plain',
  'src/PivotRoot.cxx',
  'src/util.cxx',
  'src/mime_types.cxx',
  'src/uri_escape.cxx',
  'src/expat.cxx',
  'src/wxml.cxx',
  'src/error.cxx',
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
    libwas,
    libfox,
    http_dep,
    time_dep,
    util_dep,
    io_dep,
    system_dep,
  ])

executable('davos-od',
  'src/util.cxx',
  'src/mime_types.cxx',
  'src/uri_escape.cxx',
  'src/expat.cxx',
  'src/wxml.cxx',
  'src/splice.cxx',
  'src/proppatch.cxx',
  'src/lock.cxx',
  'src/od_resource.cxx',
  'src/od_create.cxx',
  'src/od_put.cxx',
  'src/od_main.cxx',
  include_directories: inc,
  dependencies: [
    expat,
    libwas,
    libod,
    http_dep,
    time_dep,
    util_dep,
  ])

subdir('test')
