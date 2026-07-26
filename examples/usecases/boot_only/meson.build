boot_only_conf = configuration_data()
boot_only_conf.set('GDB_PORT', '3333')
boot_only_conf.set('PLATFORM_REPL', machine_platform_repl)
boot_only_conf.set('BOOT_ELF', boot_exe.full_path())
boot_only_conf.set('LOAD_APP_LINES', '')

boot_only_resc = configure_file(
  input: machine_renode_template,
  output: 'boot_only.resc',
  configuration: boot_only_conf,
)

boot_only = alias_target(
  'boot_only',
  boot_bin,
)

run_target(
  'run_boot_only',
  command: [
    'renode',
    boot_only_resc,
  ],
  depends: boot_exe,
)
