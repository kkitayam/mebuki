mbk_includes = include_directories('include')

mbk_sources = files(
  'src/mebuki.c',
  'src/taneue.c',
)

mebuki_dep = declare_dependency(
  sources: mbk_sources,
  include_directories: mbk_includes,
)

if meson.is_subproject()
  meson.override_dependency('mebuki', mebuki_dep)
endif
