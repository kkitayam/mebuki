verification = get_option('svl')
if not sign_algorithms.has_key(verification)
  error('Unsupported SVL: @0@'.format(verification))
endif

if machine_name != 'renode-cm4' and machine_name != 'rx261' and machine_name != 'rx65n' and machine_name != 'msp432e4'
  error('Unsupported machine for examples: @0@'.format(machine_name))
endif

mebuki_sign = find_program('mebuki-sign', required: true)

generations = 8
app_security_versions = [0, 1, 2]
app_key_generations = range(generations)
examples_build_dir = meson.current_build_dir()

public_keys_file = 'public_keys.h'
private_key_files = []
foreach i : range(generations)
  private_key_files += 'private_key_gen@0@.pem'.format(i)
endforeach

gen_keys = custom_target(
  'keygen',
  output: [public_keys_file] + private_key_files,
  command: [
    mebuki_sign,
    'keygen',
    '--format', 'c-array',
    '--algorithm', sign_algorithms[verification],
    '-o', '@OUTDIR@',
    '--generations', generations.to_string(),
  ],
  env: {
    'PYTHONUTF8': '1',
  },
)
public_keys = gen_keys[0]
private_keys = []
foreach i : range(generations)
  private_keys += gen_keys[i + 1]
endforeach
sign_algo = sign_algorithms[verification]

subdir('boot')
subdir('app')
if machine_name == 'renode-cm4' or machine_name == 'rx261' or machine_name == 'rx65n'
  subdir('app_ota')
endif

subdir('usecases')

if machine_name == 'renode-cm4'
  python_exe = find_program('python3', 'python', required: true)
  renode = find_program('renode', required: false)
  usecase_run_script = meson.project_source_root() / 'machines' / 'renode-cm4' / 'scripts' / 'usecase_run.py'

  if not renode.found()
    warning('renode not found. Renode use case run targets will look for it at runtime.')
  endif

  foreach usecase : all_usecases
    cmd = [
      python_exe,
      usecase_run_script,
      '--boot-image', boot_exe.full_path(),
      '--build-dir', meson.current_build_dir(),
      '--display',
    ]

    if renode.found()
      cmd += ['--renode', renode]
    endif

    if usecase['has_slot0']
      cmd += ['--slot0-image', usecase['slot0'].full_path()]
      cmd += ['--slot0-address', machine_slot0_address]
    endif

    if usecase['has_slot1']
      cmd += ['--slot1-image', usecase['slot1'].full_path()]
      cmd += ['--slot1-address', machine_slot1_address]
    endif
    if usecase['name'] == 'ota'
      cmd += [
        '--ymodem-image', app_images[2][0].full_path(),
        '--duration', '30',
      ]
    endif

    run_target(
      'run_' + usecase['name'],
      command: cmd,
      depends: usecase['depends'],
    )
  endforeach
elif machine_name == 'rx261'
  rfp_cli = find_program('rfp-cli', required: false)

  if rfp_cli.found()
    python_exe = find_program('python3', 'python', required: true)
    serial_port_option = get_option('serial_port')

    if serial_port_option == ''
      warning('serial_port option not specified. RX261 use case run targets will not be functional.')
    else
      usecase_run_script = meson.project_source_root() / 'machines' / 'rx261' / 'scripts' / 'usecase_run.py'
      run_script = meson.project_source_root() / 'machines' / 'rx261' / 'scripts' / 'run.py'

      foreach usecase : all_usecases
        cmd = [
          python_exe,
          usecase_run_script,
          serial_port_option,
          '--boot-image', boot_srec.full_path(),
          '--build-dir', meson.current_build_dir(),
          '--rfp-cli', rfp_cli,
          '--python-exe', python_exe,
          '--run-script', run_script,
          '--display',
        ]

        if usecase['has_slot0']
          cmd += ['--slot0-image', usecase['slot0'].full_path()]
          cmd += ['--slot0-address', machine_slot0_address]
        endif

        if usecase['has_slot1']
          cmd += ['--slot1-image', usecase['slot1'].full_path()]
          cmd += ['--slot1-address', machine_slot1_address]
        endif
        if usecase['name'] == 'ota'
          cmd += [
            '--ymodem-image', app_images[2][0].full_path(),
            '--duration', '30',
          ]
        endif

        run_target(
          'run_' + usecase['name'],
          command: cmd,
          depends: usecase['depends'],
        )
      endforeach
    endif
  else
    message('rfp-cli not found. RX261 use case run targets will not be available.')
  endif
elif machine_name == 'rx65n'
  rfp_cli = find_program('rfp-cli', required: false)

  if rfp_cli.found()
    python_exe = find_program('python3', 'python', required: true)
    serial_port_option = get_option('serial_port')

    if serial_port_option == ''
      warning('serial_port option not specified. RX65N use case run targets will not be functional.')
    else
      usecase_run_script = meson.project_source_root() / 'machines' / 'rx65n' / 'scripts' / 'usecase_run.py'
      run_script = meson.project_source_root() / 'machines' / 'rx65n' / 'scripts' / 'run.py'

      foreach usecase : all_usecases
        cmd = [
          python_exe,
          usecase_run_script,
          serial_port_option,
          '--boot-image', boot_dualbank_srec.full_path(),
          '--build-dir', meson.current_build_dir(),
          '--rfp-cli', rfp_cli,
          '--python-exe', python_exe,
          '--run-script', run_script,
          '--display',
        ]

        if usecase['has_slot0']
          cmd += ['--slot0-image', usecase['slot0'].full_path()]
          cmd += ['--slot0-address', machine_slot0_address]
        endif

        if usecase['has_slot1']
          cmd += ['--slot1-image', usecase['slot1'].full_path()]
          cmd += ['--slot1-address', machine_slot1_address]
        endif
        if usecase['name'] == 'ota'
          cmd += [
            '--ymodem-image', app_images[2][0].full_path(),
            '--duration', '30',
          ]
        endif

        run_target(
          'run_' + usecase['name'],
          command: cmd,
          depends: usecase['depends'],
        )
      endforeach
    endif
  else
    message('rfp-cli not found. RX65N use case run targets will not be available.')
  endif
elif machine_name == 'msp432e4'
  dslite = find_program('dslite', required: false)
  python_exe = find_program('python3', 'python', required: true)
  serial_port_option = get_option('serial_port')
  if serial_port_option == ''
    serial_port_option = 'COM5'
  endif

  if dslite.found()
    ccxml = files('../machines/msp432e4/scripts/MSP432E401Y.ccxml')
    run_script = files('../machines/msp432e4/scripts/run.py')
    boot_bank1_srec = custom_target(
      'boot_bank1_srec',
      input: boot_exe,
      output: 'boot_bank1.srec',
      command: [
        objcopy,
        '-O', 'srec',
        '--adjust-vma=0x00080000',
        '@INPUT@',
        '@OUTPUT@',
      ],
    )

    foreach usecase : all_usecases
      if usecase['name'] == 'slot0' or usecase['name'] == 'higher_version'
        slot0_srec = custom_target(
          usecase['name'] + '_slot0_srec',
          input: usecase['slot0'],
          output: usecase['name'] + '_slot0.srec',
          command: [
            objcopy,
            '-I', 'binary',
            '-O', 'srec',
            '--adjust-vma=' + machine_slot0_address,
            '@INPUT@',
            '@OUTPUT@',
          ],
        )
        slot1_srec = custom_target(
          usecase['name'] + '_slot1_srec',
          input: usecase['slot1'],
          output: usecase['name'] + '_slot1.srec',
          command: [
            objcopy,
            '-I', 'binary',
            '-O', 'srec',
            '--adjust-vma=' + machine_slot1_address,
            '@INPUT@',
            '@OUTPUT@',
          ],
        )
        run_target(
          'run_' + usecase['name'],
          command: [
            python_exe,
            run_script,
            serial_port_option,
            '--dslite', dslite,
            '--ccxml', ccxml,
            '--firmware', boot_exe,
            '--firmware', boot_bank1_srec,
            '--firmware', slot0_srec,
            '--firmware', slot1_srec,
            '--log-file', meson.current_build_dir() / ('uart_' + usecase['name'] + '.log'),
            '--expect', 'hello world',
          ],
          depends: usecase['depends'] + [boot_bank1_srec, slot0_srec, slot1_srec],
        )
      endif
    endforeach
  else
    warning('dslite not found. MSP432E4 use case run targets are unavailable.')
  endif
endif
