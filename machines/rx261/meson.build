if host_machine.cpu_family() != 'rx'
  error(f'unexpected target machine. expected cpu_family=rx / actual: {host_machine.cpu_family()}')
endif

machine_name = 'rx261'
machine_slot0_address = '0xFFF80000'
machine_slot1_address = '0xFFFA0000'
rfp_cli = find_program('rfp-cli', required: false)
machine_run_script = meson.project_source_root() / 'machines' / 'rx261' / 'scripts' / 'run.py'
machine_run_tool_args = rfp_cli.found() ? ['--rfp-cli', rfp_cli] : []
machine_run_available = rfp_cli.found()
machine_needs_serial = true

machine_hal_sources = files(
  'hal/uart.c',
  'hal/system.c',
  'hal/flash.c',
  'hal/retarget.c',
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
machine_app_startup = files('startup_app.c')
machine_boot_linker = files('linker_boot.ld')
machine_app_linker = files('linker_app.ld')
