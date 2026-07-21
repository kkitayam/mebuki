verification = get_option('svl')
if not sign_algorithms.has_key(verification)
  error('Unsupported SVL: @0@'.format(verification))
endif

generations = 8
app_security_versions = [0, 1, 2]
app_key_generations = range(generations)

mebuki_sign = find_program('mebuki-sign', required: false)
if not mebuki_sign.found()
  mebuki_sign = find_program(meson.project_source_root() / 'tools' / 'mebuki-sign.bat', required: true)
endif

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
  build_by_default: false
)

subdir('targets')
