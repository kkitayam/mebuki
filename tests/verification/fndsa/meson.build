c_fndsa_keygen_dep = dependency(
    'c-fn-dsa-keygen',
    fallback: ['c-fn-dsa', 'c_fndsa_keygen_dep'],
)

c_fndsa_sign_dep = dependency(
    'c-fn-dsa-sign',
    fallback: ['c-fn-dsa', 'c_fndsa_sign_dep'],
)

test_sources = files('test_svl.c')

test_includes = [
  include_directories('.'),
]

test_exe = executable(
  'test_fndsa',
  test_sources,
  include_directories: test_includes,
  dependencies: [unity_dep, verification_fndsa_dep, c_fndsa_keygen_dep, c_fndsa_sign_dep],
  c_args: [
    '-Wno-unused-function',
    '-Wno-unknown-pragmas',
    '-DMBK_SVL_PUBLIC_KEYS_HEADER="public_keys.h"',
  ],
  build_by_default: false,
)

test('svl_fndsa', test_exe)
