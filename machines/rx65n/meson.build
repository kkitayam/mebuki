if host_machine.cpu_family() != 'rx'
  error('unexpected target machine. expected cpu_family=rx / actual: ' + host_machine.cpu_family())
endif

if get_option('rx65n_data_location') == 'codeflash'
  add_project_arguments('-DMEBUKI_RX65N_DATA_IN_CODE_FLASH=1', language: 'c')
endif

machine_name = 'rx65n'
machine_slot0_address = '0xFFE00000'
machine_slot1_address = '0xFFF00000'

machine_hal_sources = files(
  'hal/uart.c',
  'hal/system.c',
  'hal/flash.c',
  'hal/retarget.c',
  '../../vendor/rx65n/flash_type4.c',
)

machine_hal_include = include_directories('hal')
machine_config_include = include_directories('config')
machine_vendor_include = include_directories('.', '../../vendor/rx65n')

machine_hal_dep = declare_dependency(
  sources: machine_hal_sources,
  include_directories: [machine_hal_include, machine_vendor_include],
)

machine_boot_startup = files('startup_boot.c')
machine_boot_linker = files('linker_boot.ld')
machine_app_startup = files('startup_app.c')
machine_app_linker = files('linker_app.ld')
