mebuki_dep = dependency('mebuki')
verification_dep = verification_deps[verification]
boot_library_dep = declare_dependency(
  dependencies: [
    mebuki_dep,
    verification_dep,
  ],
)

boot_sources = files(
  'main.c',
) + machine_boot_startup + public_keys

boot_c_args = []
if machine_name == 'rx65n'
  boot_c_args += ['-DMEBUKI_BOOT_USE_BANK_SWAP=1']
elif machine_name == 'msp432e4'
  boot_c_args += [
    '-DMSP432E4_STARTUP_INITIALIZES_SYSTEM=1',
    '-DMEBUKI_BOOT_USE_BANK_SWAP=1',
    '-DMEBUKI_BOOT_USE_MSP432E4_BANK_SWAP=1',
  ]
endif

boot_exe = executable(
  'boot',
  boot_sources,
  include_directories: [
    machine_config_include,
    machine_hal_include,
  ],
  dependencies: [
    mebuki_dep,
    verification_dep,
    boot_library_dep,
    machine_hal_dep,
  ],
  c_args: boot_c_args,
  link_args: [
    '-T' + machine_boot_linker[0].full_path(),
    '-Wl,-Map=' + meson.current_build_dir() / 'boot.map',
  ],
  link_depends: machine_boot_linker,
  name_suffix: 'elf',
)

boot_bin = custom_target(
  boot_exe.name(),
  input: boot_exe,
  output: boot_exe.name() + '.bin',
  command: [objcopy, '-O', 'binary', '@INPUT@', '@OUTPUT@'],
  build_by_default: false,
)

boot_srec = custom_target(
  'boot_srec',
  input: boot_exe,
  output: boot_exe.name() + '.srec',
  command: [objcopy, '-O', 'srec', '@INPUT@', '@OUTPUT@'],
  build_by_default: true,
)
machine_boot_firmwares = [boot_srec]

if machine_name == 'rx65n'
  boot_bank1_srec = custom_target(
    'boot_bank1_srec',
    input: boot_exe,
    output: 'boot_bank1.srec',
    command: [objcopy, '--change-addresses', '-0x100000', '-O', 'srec', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )

  machine_boot_firmwares += [boot_bank1_srec]
elif machine_name == 'msp432e4'
  boot_bank1_srec = custom_target(
    'boot_bank1_srec',
    input: boot_exe,
    output: 'boot_bank1.srec',
    command: [objcopy, '--change-addresses', '0x80000', '-O', 'srec', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )
  machine_boot_firmwares += [boot_bank1_srec]
endif

python_exe = find_program('python3', 'python', required: true)
serial_port = get_option('serial_port')
can_run = machine_run_available
if machine_needs_serial and serial_port == ''
  warning('serial_port not specified. Use -Dserial_port=<port>')
  can_run = false
endif

if can_run
  foreach run_name : ['run', 'run-capture']
    command = [python_exe, machine_run_script]
    if machine_needs_serial
      command += [serial_port]
    endif
    command += machine_run_tool_args
    foreach firmware : machine_boot_firmwares
      command += ['--firmware', firmware]
    endforeach
    command += [
      '--log-file', meson.current_build_dir() / ('uart_' + run_name + '.log'),
    ]
    command += machine_boot_run_args

    run_target(run_name, command: command, depends: machine_boot_firmwares)
  endforeach
endif

run_target(
  'boot_size',
  command: [
    size,
    boot_exe,
  ],
)
