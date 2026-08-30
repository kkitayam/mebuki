if host_machine.cpu() != 'cortex-m4'
  error(f'unexpected target machine. ' +
        f'expected: cortex-m4 / Actual: {host_machine.cpu()}')
endif

machine_name = 'renode-cm4'
machine_slot0_address = '0x00020000'
machine_slot1_address = '0x00040000'

machine_hal_sources = files(
  'hal/uart.c',
  'hal/flash.c',
  'hal/system.c',
  'hal/retarget.c',
)

machine_hal_include = include_directories('hal')
machine_config_include = include_directories('config')

machine_hal_dep = declare_dependency(
  sources: machine_hal_sources,
  include_directories: machine_hal_include,
)

machine_boot_startup = files('startup_boot.s')
machine_app_startup = files('startup_app.s')
machine_boot_linker = files('linker_boot.ld')
machine_app_linker = files('linker_app.ld')
