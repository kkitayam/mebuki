if host_machine.cpu_family() != 'rx'
  error(f'unexpected target machine. expected cpu_family=rx / actual: {host_machine.cpu_family()}')
endif

machine_name = 'rx261'
machine_slot0_address = '0xFFF80000'
machine_slot1_address = '0xFFFA0000'

machine_hal_sources = files(
  'hal/uart.c',
  'hal/system.c',
  'hal/flash.c',
  '../../vendor/rx261/flash_type1.c',
)

machine_hal_include = include_directories('hal')
machine_config_include = include_directories('config')
machine_vendor_include = include_directories('.', '../../vendor/rx261')

machine_hal_dep = declare_dependency(
  sources: machine_hal_sources,
  include_directories: [machine_hal_include, machine_vendor_include],
)

machine_boot_startup = files('startup_boot.c')
machine_boot_linker_file = files('linker_boot.ld')
machine_boot_linker = meson.current_source_dir() / 'linker_boot.ld'
