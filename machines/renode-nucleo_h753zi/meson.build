if host_machine.cpu() != 'cortex-m7'
  error('unexpected target machine. expected cortex-m7 / actual: ' + host_machine.cpu())
endif

machine_name = 'renode-nucleo_h753zi'
machine_slot0_address = '0x08040000'
machine_slot1_address = '0x08140000'
machine_run_script = meson.current_source_dir() / 'scripts' / 'run.py'
machine_run_tool_args = []
machine_boot_run_args = []
machine_run_available = true
machine_needs_serial = false

machine_hal_sources = files(
  'hal/uart.c',
  'hal/flash.c',
  '../renode-cm4/hal/system.c',
  '../renode-cm4/hal/retarget.c',
)
machine_hal_include = include_directories('hal', '../renode-cm4/hal')
machine_config_include = include_directories('config')
machine_hal_dep = declare_dependency(
  sources: machine_hal_sources,
  include_directories: machine_hal_include,
)

machine_boot_startup = files('startup.s')
machine_app_startup = machine_boot_startup
machine_boot_linker = files('linker_boot.ld')
machine_app_linker = files('linker_app.ld')

flash_hal_test = executable(
  'h753_flash_hal_test',
  files('tests/flash_hal_test.c') + machine_boot_startup,
  include_directories: [machine_config_include, machine_hal_include],
  dependencies: machine_hal_dep,
  link_args: ['-T' + machine_boot_linker[0].full_path()],
  link_depends: machine_boot_linker,
  name_suffix: 'elf',
  build_by_default: false,
)
flash_hal_test_srec = custom_target(
  'h753_flash_hal_test_srec',
  input: flash_hal_test,
  output: 'flash_hal_test.srec',
  command: [objcopy, '-O', 'srec', '@INPUT@', '@OUTPUT@'],
)
flash_hal_test_bank1 = custom_target(
  'h753_flash_hal_test_bank1_srec',
  input: flash_hal_test,
  output: 'flash_hal_test_bank1.srec',
  command: [objcopy, '--change-addresses', '0x100000', '-O', 'srec', '@INPUT@', '@OUTPUT@'],
)
python = find_program('python3', 'python', required: true)
run_target(
  'test_h753_flash_hal',
  command: [
    python, machine_run_script,
  ] + machine_run_tool_args + [
    '--firmware', flash_hal_test_srec,
    '--firmware', flash_hal_test_bank1,
    '--expect', 'FLASH_HAL_TEST: swap scheduled',
    '--expect', 'FLASH_HAL_TEST: PASS',
    '--log-file', meson.current_build_dir() / 'flash_hal_test.log',
  ],
  depends: [flash_hal_test_srec, flash_hal_test_bank1],
)
run_target(
  'test_h753_flash_errors',
  command: [
    python, files('tests/check_flash_errors.py'),
    flash_hal_test_srec, flash_hal_test_bank1,
  ],
  depends: [flash_hal_test_srec, flash_hal_test_bank1],
)
