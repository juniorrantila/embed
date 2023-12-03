project('embed', 'c',
  version: '0.0.1',
  license: 'BSD 2-Clause',
  meson_version: '>=0.0.1',
  default_options: [
    'c_std=c89',
    'warning_level=3',
    'buildtype=release',
  ])

subdir('src')

if get_option('examples').enabled()
  subdir('examples')
endif
