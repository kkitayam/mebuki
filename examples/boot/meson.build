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
  link_args: [
    '-T' + machine_boot_linker,
    '-Wl,-Map=' + meson.current_build_dir() / 'boot.map',
  ],
  link_depends: machine_boot_linker_file,
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
endif

run_target(
  'boot_size',
  command: [
    size,
    boot_exe,
  ],
)
