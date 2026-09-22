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
  boot_c_args += ['-DMSP432E4_STARTUP_INITIALIZES_SYSTEM=1']
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

if machine_name == 'rx261'
  rfp_cli = find_program('rfp-cli')
  python_exe = find_program('python3', 'python', required: true)
  serial_port_option = get_option('serial_port')

  boot_srec = custom_target(
    'boot_srec',
    input: boot_exe,
    output: boot_exe.name() + '.s32',
    command: [objcopy, '-O', 'srec', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )

  run_target(
    'deploy',
    command: [
      rfp_cli,
      '-d', 'RX200',
      '-t', 'e2l',
      '-if', 'fine',
      '-a',
      boot_srec,
    ],
  )

  if serial_port_option == ''
    error('serial_port option must be specified for RX261 run target. Use -Dserial_port=<port>')
  endif

  run_script = meson.project_source_root() / 'machines' / 'rx261' / 'scripts' / 'run.py'
  run_target(
    'run',
    command: [
      python_exe,
      run_script,
      serial_port_option,
      '--build-dir', meson.current_build_dir(),
      '--rfp-cli', rfp_cli,
      '--display',
    ],
  )

  run_target(
    'run-capture',
    command: [
      python_exe,
      run_script,
      serial_port_option,
      '--build-dir', meson.current_build_dir(),
      '--rfp-cli', rfp_cli,
    ],
  )
elif machine_name == 'rx65n'
  rfp_cli = find_program('rfp-cli')
  python_exe = find_program('python3', 'python', required: true)
  serial_port_option = get_option('serial_port')

  boot_srec = custom_target(
    'boot_srec',
    input: boot_exe,
    output: boot_exe.name() + '.s32',
    command: [objcopy, '-O', 'srec', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )

  boot_bank0_srec = custom_target(
    'boot_bank0_srec',
    input: boot_exe,
    output: 'boot_bank0.s32',
    command: [objcopy, '--change-addresses', '-0x100000', '-O', 'srec', '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )

  python_merge_srec = files('../../machines/rx65n/scripts/merge_srec.py')
  boot_dualbank_srec = custom_target(
    'boot_dualbank_srec',
    input: [boot_srec, boot_bank0_srec],
    output: 'boot_dualbank.s32',
    command: [python_exe, python_merge_srec, '@INPUT@', '@OUTPUT@'],
    build_by_default: true,
  )

  rfp_args = [
    '-d', 'RX65x',
    '-t', 'e2l',
    '-if', 'fine',
    '-auth', 'id', 'FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF',
  ]

  run_target(
    'deploy',
    command: [rfp_cli] + rfp_args + ['-a', boot_dualbank_srec],
  )

  if serial_port_option == ''
    error('serial_port option must be specified for RX65N run target. Use -Dserial_port=<port>')
  endif

  run_script = meson.project_source_root() / 'machines' / 'rx65n' / 'scripts' / 'run.py'
  foreach run_name : ['run', 'run-capture']
    run_args = [
      python_exe,
      run_script,
      serial_port_option,
      '--build-dir', meson.current_build_dir(),
      '--rfp-cli', rfp_cli,
    ]
    if run_name == 'run'
      run_args += ['--display']
    endif
    run_target(run_name, command: run_args)
  endforeach
elif machine_name == 'msp432e4'
  dslite = find_program('dslite', required: false)
  python_exe = find_program('python3', 'python', required: true)
  serial_port_option = get_option('serial_port')
  if serial_port_option == ''
    serial_port_option = 'COM5'
  endif

  if dslite.found()
    ccxml = files('../../machines/msp432e4/scripts/MSP432E401Y.ccxml')
    run_script = files('../../machines/msp432e4/scripts/run.py')

    run_target(
      'deploy',
      command: [dslite, '-c', ccxml, '-u', boot_exe],
    )
    run_target(
      'run-capture',
      command: [
        python_exe,
        run_script,
        serial_port_option,
        '--dslite', dslite,
        '--ccxml', ccxml,
        '--firmware', boot_exe,
        '--log-file', meson.current_build_dir() / 'uart.log',
        '--expect', 'No bootable slot found',
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
