project('mebuki', 'c',
  version : '0.1.0',
  meson_version : '>=1.10.0',
  default_options : ['c_std=c23', 'buildtype=minsize']
)

build_examples = meson.is_cross_build()
build_tests = not meson.is_cross_build()

subdir('mebuki')
subdir('verification')

if not meson.is_subproject()
  if build_examples
    subdir('machines')
    subdir('examples')
  endif
  if build_tests
    subdir('tests')
  endif
endif
