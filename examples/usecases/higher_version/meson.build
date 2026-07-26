higher_version_conf = configuration_data()
higher_version_conf.set('GDB_PORT', '3333')
higher_version_conf.set('PLATFORM_REPL', machine_platform_repl)
higher_version_conf.set('BOOT_ELF', boot_exe.full_path())
higher_version_slot0_image_path = app_images[1][1].full_path()
higher_version_slot1_image_path = app_images[2][0].full_path()
higher_version_conf.set(
  'LOAD_APP_LINES',
  'sysbus LoadBinary @' + higher_version_slot0_image_path + ' ' + machine_slot0_address + '\n' +
  'sysbus LoadBinary @' + higher_version_slot1_image_path + ' ' + machine_slot1_address,
)

higher_version_resc = configure_file(
  input: machine_renode_template,
  output: 'higher_version.resc',
  configuration: higher_version_conf,
)

run_target(
  'run_higher_version',
  command: [
    'renode',
    higher_version_resc,
  ],
  depends: [boot_exe, app_images[2][0], app_images[1][1]],
)
