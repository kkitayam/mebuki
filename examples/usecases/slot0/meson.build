slot0_conf = configuration_data()
slot0_conf.set('GDB_PORT', '3333')
slot0_conf.set('PLATFORM_REPL', machine_platform_repl)
slot0_conf.set('BOOT_ELF', boot_exe.full_path())
slot0_slot0_image_path = app_images[1][0].full_path()
slot0_slot1_image_path = app_images[1][1].full_path()
slot0_conf.set(
  'LOAD_APP_LINES',
  'sysbus LoadBinary @' + slot0_slot0_image_path + ' ' + machine_slot0_address + '\n' +
  'sysbus LoadBinary @' + slot0_slot1_image_path + ' ' + machine_slot1_address,
)

slot0_resc = configure_file(
  input: machine_renode_template,
  output: 'slot0.resc',
  configuration: slot0_conf,
)

run_target(
  'run_slot0',
  command: [
    'renode',
    slot0_resc,
  ],
  depends: [boot_exe, app_images[1][1], app_images[1][0]],
)
