rsip_sources = files(
  'mbk_svl.c',
  '../../vendor/rx261/rsip_ecdsa_p256_sha256.c',
)

rsip_includes = include_directories('.', '../../vendor/rx261')

verification_ecdsa_rsip_dep = declare_dependency(
  sources: rsip_sources,
  include_directories: [mbk_svl_includes, rsip_includes],
)

if meson.is_subproject()
  meson.override_dependency('mebuki', verification_ecdsa_rsip_dep)
endif
