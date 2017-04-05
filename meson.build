project('CM4all Davos', ['c', 'cpp'], version: '0.9',
  default_options: [
    'c_std=c99',
    'cpp_std=c++14'
  ],
)

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

libinline = dependency('libcm4all-inline', version: '>= 0.3.5')
libwas = dependency('libcm4all-was-simple', version: '>= 1.9')
libfox = dependency('libcm4all-fox', version: '>= 2.0')
libod = dependency('libcm4all-od-setup', version: '>= 1.2')

inc = include_directories('src', 'libcommon/src')

util = static_library('util',
  'libcommon/src/util/PrintException.cxx',
  'libcommon/src/util/StringView.cxx',
  'src/util/UriEscape.cxx',
  'src/gmtime.c',
  'src/date.c',
  'src/format.c',
  include_directories: inc,
  dependencies: [
    libinline
  ])
util_dep = declare_dependency(link_with: util)

http = static_library('http',
  'libcommon/src/http/List.cxx',
  include_directories: inc,
  dependencies: [
    libinline,
    util_dep,
  ])
http_dep = declare_dependency(link_with: http)

io = static_library('io',
  'libcommon/src/io/FileDescriptor.cxx',
  'libcommon/src/io/FileWriter.cxx',
  include_directories: inc,
  dependencies: [
    libinline
  ])
io_dep = declare_dependency(link_with: io)

executable('davos-plain',
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
    libinline,
    libwas,
    libfox,
    util_dep,
    http_dep,
    io_dep,
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
    libinline,
    libwas,
    libod,
    util_dep,
  ])

test('t_uri_escape', executable('t_uri_escape',
  'test/t_uri_escape.cxx',
  include_directories: inc,
  install: false,
  dependencies: [
    libinline,
    util_dep,
  ]))
