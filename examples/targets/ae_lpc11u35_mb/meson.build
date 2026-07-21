# ============================================================================
# ターゲット: ae_lpc11u35_mb (Akizuki AE LPC11U35-MB)
# ============================================================================

if host_machine.cpu() != 'cortex-m0'
  error('想定していないターゲットマシンです。期待: cortex-m0 / 実際: ' + host_machine.cpu())
endif

# ============================================================================
# 依存ライブラリ
# ============================================================================

mebuki_dep = dependency('mebuki', fallback: ['mebuki', 'mebuki_dep'])
mebuki_svl_dep = dependency(
  'mebuki-svl-monocypher',
  fallback: ['mebuki-svl-monocypher', 'mebuki_svl_dep'],
)
cmsis_6_dep = dependency('cmsis-6', fallback: ['cmsis-6', 'cmsis_6_dep'])
nxp_driver_lpc11uxx_dep = dependency(
  'nxp-driver-lpc11uxx',
  fallback: ['nxp-driver-lpc11uxx', 'nxp_driver_lpc11uxx_dep'],
)

# ============================================================================
# HAL ライブラリ (ターゲット固有)
# ============================================================================

hal_sources = files(
  'hal/uart.c',
  'hal/flash.c',
  'hal/system.c',
  'hal/retarget.c',
)

hal_includes = include_directories('hal')

hal_dep = declare_dependency(
  sources: hal_sources,
  include_directories: [hal_includes, include_directories('config')],
  dependencies: [cmsis_6_dep, nxp_driver_lpc11uxx_dep],
)

# ============================================================================
# Boot Software 実行ファイル
# ============================================================================

boot_sources = files(
  '../../boot/main.c',
  'startup_boot.s',
)

boot_exe = executable(
  'boot',
  boot_sources + gen_keys[0],
  include_directories: [
    include_directories('config'),
    include_directories('hal'),
  ],
  dependencies: [
    mebuki_dep,
    mebuki_svl_dep,
    hal_dep,
  ],
  link_args: [
    '-T' + meson.current_source_dir() / 'linker_boot.ld',
    '-Wl,-Map=' + meson.current_build_dir() / 'boot.map',
  ],
  link_depends: files('linker_boot.ld'),
  name_suffix: 'elf',
)

app_sources = files(
  '../../app/main.c',
  'startup_app.s',
)

app_exe = executable(
  'app',
  app_sources,
  include_directories: [
    include_directories('config'),
    include_directories('hal'),
  ],
  dependencies: [
    hal_dep,
  ],
  link_args: [
    '-T' + meson.current_source_dir() / 'linker_app.ld',
    '-Wl,-Map=' + meson.current_build_dir() / 'app.map',
  ],
  link_depends: files('linker_app.ld'),
  name_suffix: 'elf',
)

# ============================================================================
# バイナリ生成（objcopy）
# ============================================================================

objcopy = find_program('objcopy')
boot_bin = custom_target(
  boot_exe.name(),
  input: boot_exe,
  output: boot_exe.name() + '.bin',
  command: [objcopy, '-O', 'binary', '@INPUT@', '@OUTPUT@'],
  build_by_default: true,
)
app_bin = custom_target(
  app_exe.name(),
  input: app_exe,
  output: app_exe.name() + '.bin',
  command: [objcopy, '-O', 'binary', '@INPUT@', '@OUTPUT@'],
  build_by_default: true,
)

app_image_targets = []
app_image_versions = []

foreach security_version : app_security_versions
  version_image_targets = []

  foreach key_generation : app_key_generations
    app_image = custom_target(
      'app_v@0@_k@1@'.format(security_version, key_generation),
      input: [app_bin, gen_keys[key_generation + 1]],
      output: 'app.v@0@.k@1@.img'.format(security_version, key_generation),
      command: [
        mebuki_sign,
        'sign',
        '--algorithm', 'eddsa-25519-blake2b',
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
      build_by_default: false,
    )

    app_image_targets += [app_image]
    version_image_targets += [app_image]
  endforeach

  app_image_versions += [version_image_targets]
endforeach

ae_app_images = alias_target('ae_app_images', app_image_targets)

# ============================================================================
# サイズ表示（arm-none-eabi-size）
# ============================================================================

size = find_program('size')
run_target(
  'boot_size',
  command: [
    size,
    boot_exe,
  ],
  depends: boot_exe,
)

run_target(
  'app_size',
  command: [
    size,
    app_exe,
  ],
  depends: app_exe,
)