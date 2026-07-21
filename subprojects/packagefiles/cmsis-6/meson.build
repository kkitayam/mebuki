project('cmsis-6', 'c',
  version: '6.3.0',
  meson_version: '>=1.10.0',
  default_options: ['c_std=c23']
)

cmsis_6_dep = declare_dependency(
  include_directories: include_directories('CMSIS/Core/Include'),
)

if meson.is_subproject()
  meson.override_dependency('cmsis-6', cmsis_6_dep)
endif
