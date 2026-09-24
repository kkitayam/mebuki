project('cmsis', 'c', version : '6.3.0')

cmsis_inc = include_directories(
  'CMSIS/Core/Include',
  'CMSIS/Driver/Include',
  'CMSIS/RTOS2/Include',
)

cmsis_dep = declare_dependency(
 include_directories : cmsis_inc,
)

meson.override_dependency('cmsis', cmsis_dep)
