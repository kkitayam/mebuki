bearssl_dep = dependency(
    'bearssl',
    fallback: ['bearssl', 'bearssl_dep'],
)

ecdsa_sources = files('mbk_svl.c')
ecdsa_includes = include_directories('.')

verification_ecdsa_dep = declare_dependency(
  sources: ecdsa_sources,
  include_directories: [mbk_svl_includes, ecdsa_includes],
  dependencies: [bearssl_dep],
)

if meson.is_subproject()
  meson.override_dependency('mebuki', verification_ecdsa_dep)
endif
