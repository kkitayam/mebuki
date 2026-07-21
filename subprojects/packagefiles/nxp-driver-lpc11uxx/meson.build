project('nxp-driver-lpc11uxx', 'c',
  version: '0.0.0',
  meson_version: '>=1.10.0',
  default_options: ['c_std=c23']
)

cmsis_6_dep = dependency('cmsis-6', fallback: ['cmsis-6', 'cmsis_6_dep'])

nxp_driver_lpc11uxx_inc = include_directories('lpcopen/lpc11uxx/lpc_chip_11uxx/inc')

nxp_driver_lpc11uxx_dep = declare_dependency(
  include_directories: nxp_driver_lpc11uxx_inc,
  dependencies: [cmsis_6_dep],
  compile_args: [
    '-DCORE_M0',
  ],
  sources: files(
    'lpcopen/lpc11uxx/lpc_chip_11uxx/src/clock_11xx.c',
    'lpcopen/lpc11uxx/lpc_chip_11uxx/src/iocon_11xx.c',
    'lpcopen/lpc11uxx/lpc_chip_11uxx/src/uart_11xx.c',
  ),
)

if meson.is_subproject()
  meson.override_dependency('nxp-driver-lpc11uxx', nxp_driver_lpc11uxx_dep)
endif