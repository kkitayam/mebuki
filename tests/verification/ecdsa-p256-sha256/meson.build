test_exe = executable(
  'test_svl',
  files('test_svl.c'),
  include_directories: include_directories('.'),
  dependencies: [unity_dep, verification_ecdsa_dep],
  c_args: [
    '-Wno-unused-function',
    '-Wno-unknown-pragmas',
    '-DMBK_SVL_PUBLIC_KEYS_HEADER="public_keys.h"',
  ],
  build_by_default: false,
)

test('svl_ecdsa_p256_sha256', test_exe)
