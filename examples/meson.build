verification = get_option('svl')
if not sign_algorithms.has_key(verification)
  error('Unsupported SVL: @0@'.format(verification))
endif

if machine_name != 'renode-cm4' and machine_name != 'rx261'
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

if machine_name == 'renode-cm4'
  subdir('app')
  subdir('usecases')
endif
