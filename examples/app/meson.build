app_sources = files(
  'main.c',
) + machine_app_startup

app_exe = executable(
  'app',
  app_sources,
  include_directories: [
    machine_config_include,
    machine_hal_include,
  ],
  dependencies: [
    machine_hal_dep,
  ],
  link_args: [
    '-T' + machine_app_linker,
    '-Wl,-Map=' + meson.current_build_dir() / 'app.map',
  ],
  link_depends: machine_app_linker_file,
  name_suffix: 'elf',
)

app_bin = custom_target(
  app_exe.name(),
  input: app_exe,
  output: app_exe.name() + '.bin',
  command: [objcopy, '-O', 'binary', '@INPUT@', '@OUTPUT@'],
)

app_image_targets = []
app_images = []

foreach security_version : app_security_versions
  version_image_targets = []

  foreach key_generation : app_key_generations
    app_image = custom_target(
      'app_v@0@_k@1@'.format(security_version, key_generation),
      input: [app_bin, private_keys[key_generation]],
      output: 'app.v@0@.k@1@.img'.format(security_version, key_generation),
      command: [
        mebuki_sign,
        'sign',
        '--algorithm', sign_algo,
        '--key-format', 'pem',
        '-k', '@INPUT1@',
        '-i', '@INPUT0@',
        '-o', '@OUTPUT@',
        '-v', security_version.to_string(),
        '-g', key_generation.to_string(),
      ],
      env: {
        'PYTHONUTF8': '1',
      },
      build_by_default: true,
    )

    app_image_targets += [app_image]
    version_image_targets += [app_image]
  endforeach

  app_images += [version_image_targets]
endforeach

run_target(
  'app_size',
  command: [
    size,
    app_exe,
  ],
)
