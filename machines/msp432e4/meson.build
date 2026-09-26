if host_machine.cpu() != 'cortex-m4'
  error('unexpected target machine. expected cortex-m4 / actual: ' + host_machine.cpu())
endif

machine_name = 'msp432e4'
machine_slot0_address = '0x00020000'
machine_slot1_address = '0x000A0000'
dslite = find_program('dslite', required: false)
machine_run_script = meson.project_source_root() / 'machines' / 'msp432e4' / 'scripts' / 'run.py'
machine_run_tool_args = []
machine_boot_run_args = ['--no-deploy']
if dslite.found()
  machine_run_tool_args = [
    '--dslite', dslite,
    '--ccxml', files('scripts/MSP432E401Y.ccxml'),
  ]
endif
machine_run_available = dslite.found()
machine_needs_serial = true

machine_hal_sources = files(
  'hal/uart.c',
  'hal/system.c',
  'hal/flash.c',
  'hal/retarget.c',
  '../../vendor/msp432e4/msp432e401y_flash.c',
)

machine_hal_include = include_directories('hal')
machine_config_include = include_directories('config')
machine_vendor_include = include_directories('../../vendor/msp432e4')
cmsis_dep = dependency('cmsis')

machine_hal_dep = declare_dependency(
  sources: machine_hal_sources,
  dependencies: cmsis_dep,
  include_directories: [machine_hal_include, machine_vendor_include],
)

machine_boot_startup = files('startup_boot.c')
machine_app_startup = files('startup_app.c')
machine_boot_linker = files('linker_boot.ld')
machine_app_linker = files('linker_app.ld')
