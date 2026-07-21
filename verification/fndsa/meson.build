c_fndsa_common_dep = dependency(
    'c-fn-dsa-common',
    fallback: ['c-fn-dsa', 'c_fndsa_common_dep'],
)

c_fndsa_vrfy_dep = dependency(
    'c-fn-dsa-vrfy',
    fallback: ['c-fn-dsa', 'c_fndsa_vrfy_dep'],
)

fndsa_sources = files(
  'mbk_svl.c',
  '../../vendor/blake2/blake2s-ref.c',
)

fndsa_includes = include_directories('.', '../../vendor/blake2')

verification_fndsa_dep = declare_dependency(
  sources: fndsa_sources,
  include_directories: fndsa_includes,
  dependencies: [c_fndsa_common_dep, c_fndsa_vrfy_dep],
)

if meson.is_subproject()
  meson.override_dependency('mebuki', verification_fndsa_dep)
endif
