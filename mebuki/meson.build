mbk_includes = include_directories('include')

mbk_sources = files('src/mebuki.c')
if get_option('target') != 'rx65n'
  mbk_sources += files('src/taneue.c')
endif

mbk_svl_includes = include_directories('src')

mebuki_dep = declare_dependency(
  sources: mbk_sources,
  include_directories: mbk_includes,
)

meson.override_dependency('mebuki', mebuki_dep)
