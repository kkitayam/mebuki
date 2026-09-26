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
if machine_name == 'renode-cm4' or machine_name == 'rx261' or machine_name == 'rx65n' or machine_name == 'msp432e4'
  subdir('app_ota')
endif

subdir('usecases')

python_exe = find_program('python3', 'python', required: true)
serial_port = get_option('serial_port')
can_run = machine_run_available
if machine_needs_serial and serial_port == ''
  warning('serial_port not specified. Use -Dserial_port=<port>')
  can_run = false
endif

if can_run
  foreach usecase : all_usecases
    firmwares = []
    depends = usecase['depends']
    foreach firmware : machine_boot_firmwares
      firmwares += ['--firmware', firmware]
      depends += [firmware]
    endforeach

    if usecase['has_slot0']
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
      firmwares += ['--firmware', slot0_srec]
      depends += [slot0_srec]
    endif

    if usecase['has_slot1']
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
      firmwares += ['--firmware', slot1_srec]
      depends += [slot1_srec]
    endif

    extra = []
    if usecase['name'] == 'ota'
      extra = [
        '--ymodem-image', app_images[2][0],
        '--duration', '30',
      ]
    endif

    cmd = [python_exe, machine_run_script]
    if machine_needs_serial
      cmd += [serial_port]
    endif
    cmd += machine_run_tool_args + firmwares + [
      '--log-file', meson.current_build_dir() / ('uart_' + usecase['name'] + '.log'),
    ] + extra

    run_target(
      'run_' + usecase['name'],
      command: cmd,
      depends: depends,
    )
  endforeach
endif
