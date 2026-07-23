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

run_target(
  'boot_size',
  command: [
    size,
    boot_exe,
  ],
)
