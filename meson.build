project('CM4all Davos', ['c', 'cpp'], version: '0.15',
  default_options: [
    'c_std=c11',
    'cpp_std=c++17',
    'warning_level=3',
  ],
)

compiler = meson.get_compiler('cpp')

common_flags = [
  '-D_GNU_SOURCE',
]

add_global_arguments(common_flags,
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
  '-Wwrite-strings', '-Wcast-qual', '-Wcast-align', '-Wfloat-equal',
  '-Wshadow', '-Wpointer-arith', '-Wsign-compare',
  '-Wmissing-declarations', '-Wmissing-noreturn', '-Wmissing-format-attribute',
  '-Wredundant-decls', '-Wno-long-long', '-Wundef',
  '-Wno-missing-field-initializers',
  '-Wno-non-virtual-dtor',
  language: 'cpp')

expat = dependency('expat')

libwas = dependency('libcm4all-was-simple', version: '>= 1.9')
libfox = dependency('libcm4all-fox', version: '>= 2.0', required: get_option('plain'), disabler: true)
libod = dependency('libcm4all-od-setup', version: '>= 1.2', required: get_option('od'), disabler: true)

inc = include_directories('src', 'libcommon/src')

libcommon_enable_libcap = false
libcommon_enable_filesystem = false

subdir('libcommon/src/util')

util2 = static_library(
  'util2',
  'src/util/UriEscape.cxx',
  include_directories: inc,
)
util_dep = declare_dependency(
  link_with: util2,
  dependencies: [
    util_dep,
  ],
)

subdir('libcommon/src/time')
subdir('libcommon/src/http')
subdir('libcommon/src/io')
subdir('libcommon/src/system')

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
    spawn_dep,
  ],
  install: true,
  install_dir: 'lib/cm4all/was/bin',
)

executable(
  'davos-od',
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
  ],
  install: true,
  install_dir: 'lib/cm4all/was/bin',
)

subdir('test')
subdir('doc')
