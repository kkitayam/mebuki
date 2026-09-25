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

if machine_name == 'renode-cm4'
  boot_srec = custom_target(
    'boot_srec',
    input: boot_exe,
    output: boot_exe.name() + '.srec',
    command: [objcopy, '-O', 'srec', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )
  machine_boot_firmwares = [boot_srec]
elif machine_name == 'rx261'
  rfp_cli = find_program('rfp-cli', required: false)
  python_exe = find_program('python3', 'python', required: true)
  serial_port_option = get_option('serial_port')

  boot_srec = custom_target(
    'boot_srec',
    input: boot_exe,
    output: boot_exe.name() + '.srec',
    command: [objcopy, '-O', 'srec', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )

  machine_boot_firmwares = [boot_srec]

  if rfp_cli.found() and serial_port_option != ''
    run_script = meson.project_source_root() / 'machines' / 'rx261' / 'scripts' / 'run.py'
    run_target(
      'run',
      command: [
        python_exe,
        run_script,
        serial_port_option,
        '--rfp-cli', rfp_cli,
        '--log-file', meson.current_build_dir() / 'uart.log',
        '--no-deploy',
        '--display',
      ],
    )

    run_target(
      'run-capture',
      command: [
        python_exe,
        run_script,
        serial_port_option,
        '--rfp-cli', rfp_cli,
        '--log-file', meson.current_build_dir() / 'uart.log',
      ],
    )
  endif
elif machine_name == 'rx65n'
  rfp_cli = find_program('rfp-cli', required: false)
  python_exe = find_program('python3', 'python', required: true)
  serial_port_option = get_option('serial_port')

  boot_bank1_srec = custom_target(
    'boot_bank1_srec',
    input: boot_exe,
    output: 'boot_bank1.srec',
    command: [objcopy, '-O', 'srec', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )

  boot_bank0_srec = custom_target(
    'boot_bank0_srec',
    input: boot_exe,
    output: 'boot_bank0.srec',
    command: [objcopy, '--change-addresses', '-0x100000', '-O', 'srec', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )

  machine_boot_firmwares = [boot_bank1_srec, boot_bank0_srec]

  if rfp_cli.found() and serial_port_option != ''
    run_script = meson.project_source_root() / 'machines' / 'rx65n' / 'scripts' / 'run.py'
    foreach run_name : ['run', 'run-capture']
      run_args = [
        python_exe,
        run_script,
        serial_port_option,
        '--rfp-cli', rfp_cli,
        '--log-file', meson.current_build_dir() / 'uart.log',
        '--no-deploy',
      ]
      if run_name == 'run'
        run_args += ['--display']
      endif
      run_target(run_name, command: run_args)
    endforeach
  endif
elif machine_name == 'msp432e4'
  dslite = find_program('dslite', required: false)
  python_exe = find_program('python3', 'python', required: true)
  serial_port_option = get_option('serial_port')
  boot_srec = custom_target(
    'boot_srec',
    input: boot_exe,
    output: boot_exe.name() + '.srec',
    command: [objcopy, '-O', 'srec', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )
  boot_bank1_srec = custom_target(
    'boot_bank1_srec',
    input: boot_exe,
    output: 'boot_bank1.srec',
    command: [
      objcopy,
      '-O', 'srec',
      '--adjust-vma=0x00080000',
      '@INPUT@',
      '@OUTPUT@',
    ],
    build_by_default: true,
  )
  machine_boot_firmwares = [boot_srec, boot_bank1_srec]
  if serial_port_option == ''
    warning('serial_port option not specified. Use -Dserial_port=<port>')
  elif dslite.found()
    ccxml = files('../../machines/msp432e4/scripts/MSP432E401Y.ccxml')
    run_script = files('../../machines/msp432e4/scripts/run.py')

    run_target(
      'run',
      command: [
        python_exe,
        run_script,
        serial_port_option,
        '--dslite', dslite,
        '--ccxml', ccxml,
        '--log-file', meson.current_build_dir() / 'uart.log',
        '--no-deploy',
      ],
    )
    run_target(
      'run-capture',
      command: [
        python_exe,
        run_script,
        serial_port_option,
        '--dslite', dslite,
        '--ccxml', ccxml,
        '--log-file', meson.current_build_dir() / 'uart.log',
        '--no-deploy',
      ],
    )
  else
    warning('dslite not found. MSP432E4 deploy targets are unavailable.')
  endif
endif

run_target(
  'boot_size',
  command: [
    size,
    boot_exe,
  ],
)
