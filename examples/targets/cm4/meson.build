if host_machine.cpu() != 'cortex-m4'
  error(f'unexpected target machine. ' +
        f'expected: cortex-m4 / Actual: {host_machine.cpu_family()}')
endif

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
  include_directories: hal_includes,
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
    verification_deps[verification],
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
        '--algorithm', sign_algorithms[verification],
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

cm4_app_images = alias_target('cm4_app_images', app_image_targets)

app_v0_k0 = app_image_versions[0][0]
app_v0_k7 = app_image_versions[0][7]
app_v1_k0 = app_image_versions[1][0]
app_v1_k7 = app_image_versions[1][7]
app_v2_k0 = app_image_versions[2][0]

renode_same_version_slot0_image = 'app.v@0@.k@1@.img'.format(1, 7)
renode_same_version_slot1_image = 'app.v@0@.k@1@.img'.format(1, 0)
renode_higher_version_slot0_image = 'app.v@0@.k@1@.img'.format(2, 0)
renode_higher_version_slot1_image = 'app.v@0@.k@1@.img'.format(1, 7)
renode_keygen_mix_slot0_image = 'app.v@0@.k@1@.img'.format(0, 7)
renode_keygen_mix_slot1_image = 'app.v@0@.k@1@.img'.format(0, 0)

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

# ============================================================================
# Renode 起動
# ============================================================================

cmd = find_program('cmd', native: true, required: true)

configure_file(
  input: 'renode/platform.repl',
  output: 'platform.repl',
  copy: true,
)

renode_same_version_conf = configuration_data()
renode_same_version_conf.set('GDB_PORT', '3333')
renode_same_version_conf.set(
  'LOAD_APP_LINES',
  'sysbus LoadBinary @' + renode_same_version_slot0_image + ' 0x00020000\n' +
  'sysbus LoadBinary @' + renode_same_version_slot1_image + ' 0x00040000',
)

renode_slot0_resc = configure_file(
  input: 'renode/renode_common.resc.in',
  output: 'renode_cm4_slot0.resc',
  configuration: renode_same_version_conf,
)

renode_higher_version_conf = configuration_data()
renode_higher_version_conf.set('GDB_PORT', '3333')
renode_higher_version_conf.set(
  'LOAD_APP_LINES',
  'sysbus LoadBinary @' + renode_higher_version_slot0_image + ' 0x00020000\n' +
  'sysbus LoadBinary @' + renode_higher_version_slot1_image + ' 0x00040000',
)

renode_higher_version_resc = configure_file(
  input: 'renode/renode_common.resc.in',
  output: 'renode_cm4_slot0_higher_version.resc',
  configuration: renode_higher_version_conf,
)

renode_keygen_mix_conf = configuration_data()
renode_keygen_mix_conf.set('GDB_PORT', '3333')
renode_keygen_mix_conf.set(
  'LOAD_APP_LINES',
  'sysbus LoadBinary @' + renode_keygen_mix_slot0_image + ' 0x00020000\n' +
  'sysbus LoadBinary @' + renode_keygen_mix_slot1_image + ' 0x00040000',
)

renode_keygen_mix_resc = configure_file(
  input: 'renode/renode_common.resc.in',
  output: 'renode_cm4_slot0_keygen_mix.resc',
  configuration: renode_keygen_mix_conf,
)

renode_boot_only_conf = configuration_data()
renode_boot_only_conf.set('GDB_PORT', '3333')
renode_boot_only_conf.set('LOAD_APP_LINES', '')

renode_boot_only_resc = configure_file(
  input: 'renode/renode_common.resc.in',
  output: 'renode_cm4_boot_only.resc',
  configuration: renode_boot_only_conf,
)

run_target(
  'renode_cm4_slot0',
  command: [
    cmd,
    '/c',
    'renode',
    '--disable-xwt',
    renode_slot0_resc,
  ],
  depends: [boot_exe, app_v1_k7, app_v1_k0],
)

run_target(
  'renode_cm4_slot0_higher_version',
  command: [
    cmd,
    '/c',
    'renode',
    '--disable-xwt',
    renode_higher_version_resc,
  ],
  depends: [boot_exe, app_v2_k0, app_v1_k7],
)

run_target(
  'renode_cm4_slot0_keygen_mix',
  command: [
    cmd,
    '/c',
    'renode',
    '--disable-xwt',
    renode_keygen_mix_resc,
  ],
  depends: [boot_exe, app_v0_k7, app_v0_k0],
)

run_target(
  'renode_cm4_boot_only',
  command: [
    cmd,
    '/c',
    'renode',
    '--disable-xwt',
    renode_boot_only_resc,
  ],
  depends: boot_exe,
)
