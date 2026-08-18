bearssl_dep = dependency(
    'bearssl',
    fallback: ['bearssl', 'bearssl_dep'],
)

accel_option = get_option('accel')
target_option = get_option('target')
rsip_available = target_option == 'rx261'

if accel_option == 'enabled' and not rsip_available
  error('accel=enabled requires target=rx261')
endif

ecdsa_sources = files('mbk_svl.c')
ecdsa_includes = include_directories('.')
ecdsa_compile_args = []

if accel_option == 'disabled'
  ecdsa_compile_args += '-DMBK_SVL_ACCEL_MODE=0'
elif accel_option == 'auto'
  ecdsa_compile_args += '-DMBK_SVL_ACCEL_MODE=1'
else
  ecdsa_compile_args += '-DMBK_SVL_ACCEL_MODE=2'
endif

if rsip_available and accel_option != 'disabled'
  ecdsa_sources += files(
    '../../vendor/rx261/rsip_ecdsa_p256_sha256.c',
  )
  ecdsa_compile_args += '-DMBK_SVL_RSIP_AVAILABLE=1'
  ecdsa_includes = include_directories('.', '../../vendor/rx261')
endif

verification_ecdsa_dep = declare_dependency(
  sources: ecdsa_sources,
  include_directories: [mbk_svl_includes, ecdsa_includes],
  dependencies: [bearssl_dep],
  compile_args: ecdsa_compile_args,
)

if meson.is_subproject()
  meson.override_dependency('mebuki', verification_ecdsa_dep)
endif
