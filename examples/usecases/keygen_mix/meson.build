keygen_mix_conf = configuration_data()
keygen_mix_conf.set('GDB_PORT', '3333')
keygen_mix_conf.set('PLATFORM_REPL', machine_platform_repl)
keygen_mix_conf.set('BOOT_ELF', boot_exe.full_path())
keygen_mix_slot0_image_path = app_images[0][1].full_path()
keygen_mix_slot1_image_path = app_images[0][0].full_path()
keygen_mix_conf.set(
  'LOAD_APP_LINES',
  'sysbus LoadBinary @' + keygen_mix_slot0_image_path + ' ' + machine_slot0_address + '\n' +
  'sysbus LoadBinary @' + keygen_mix_slot1_image_path + ' ' + machine_slot1_address,
)

keygen_mix_resc = configure_file(
  input: machine_renode_template,
  output: 'keygen_mix.resc',
  configuration: keygen_mix_conf,
)

run_target(
  'run_keygen_mix',
  command: [
    'renode',
    keygen_mix_resc,
  ],
  depends: [boot_exe, app_images[0][1], app_images[0][0]],
)
