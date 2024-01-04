gtest = dependency('gtest', main: true, fallback: ['gtest', 'gtest_main_dep'])

test('t_uri_escape', executable('t_uri_escape',
  't_uri_escape.cxx',
  include_directories: inc,
  install: false,
  dependencies: [
    gtest,
    util_dep,
  ]))
