app_ota_sources = files(
  'main.c',
  'ymodem/ymodem.c',
) + machine_app_startup

app_ota_exe = executable(
  'app_ota',
  app_ota_sources,
  include_directories: [
    machine_config_include,
    machine_hal_include,
    include_directories('ymodem'),
  ],
  dependencies: [
    machine_hal_dep,
  ],
  link_args: [
    '-T' + machine_app_linker[0].full_path(),
    '-Wl,-Map=' + meson.current_build_dir() / 'app_ota.map',
  ],
  link_depends: machine_app_linker,
  c_args: machine_name == 'msp432e4' ? ['-DMSP432E4_STARTUP_INITIALIZES_SYSTEM=1'] : [],
  name_suffix: 'elf',
)

app_ota_bin = custom_target(
  app_ota_exe.name(),
  input: app_ota_exe,
  output: app_ota_exe.name() + '.bin',
  command: [objcopy, '-O', 'binary', '@INPUT@', '@OUTPUT@'],
)

app_ota_image = custom_target(
  'app_ota_v1_k0',
  input: [app_ota_bin, private_keys[0]],
  output: 'app_ota.v1.k0.img',
  command: [
    mebuki_sign,
    'sign',
    '--algorithm', sign_algo,
    '--key-format', 'pem',
    '-k', '@INPUT1@',
    '-i', '@INPUT0@',
    '-o', '@OUTPUT@',
    '-v', '1',
    '-g', '0',
  ],
  env: {
    'PYTHONUTF8': '1',
  },
  build_by_default: true,
)
