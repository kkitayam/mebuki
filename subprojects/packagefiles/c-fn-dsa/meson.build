project('c-fn-dsa', 'c',
  version: '0.0.0',
  meson_version: '>=1.10.0'
)

compile_args = []

c_fndsa_common_sources = files(
  'codec.c',
  'sha3.c',
  'mq.c',
  'sysrng.c',
  'util.c',
)
c_fndsa_keygen_sources = files(
  'kgen.c',
  'kgen_fxp.c',
  'kgen_gauss.c',
  'kgen_mp31.c',
  'kgen_ntru.c',
  'kgen_poly.c',
  'kgen_zint31.c',
)
c_fndsa_sign_sources = files(
  'sign.c',
  'sign_core.c',
  'sign_fpoly.c',
  'sign_fpr.c',
  'sign_sampler.c',
)
c_fndsa_vrfy_sources = files(
  'vrfy.c',
)
is_cm4 = host_machine.cpu().startswith('cortex-m4')
has_fpu = meson.get_external_property('has_fpu', false)
cc = meson.get_compiler('c').get_id()
if is_cm4 and has_fpu and (cc == 'gcc' or cc == 'clang')
  compile_args += '-DFNDSA_ASM_CORTEXM4=1'
  c_fndsa_common_sources += files(
    'codec_cm4.s',
    'sha3_cm4.s',
    'mq_cm4.s',
  )
  c_fndsa_sign_sources += files(
    'sign_fpr_cm4.s',
    'sign_sampler_cm4.s',
  )
endif
c_fndsa_sources = c_fndsa_common_sources + c_fndsa_keygen_sources + c_fndsa_sign_sources + c_fndsa_vrfy_sources

c_fndsa_common_dep = declare_dependency(
  sources: c_fndsa_common_sources,
  compile_args : compile_args,
  include_directories: include_directories('.'),
)

c_fndsa_keygen_dep = declare_dependency(
  sources: c_fndsa_keygen_sources,
  include_directories: include_directories('.'),
)

c_fndsa_sign_dep = declare_dependency(
  sources: c_fndsa_sign_sources,
  compile_args : compile_args,
  include_directories: include_directories('.'),
)

c_fndsa_vrfy_dep = declare_dependency(
  sources: c_fndsa_vrfy_sources,
  include_directories: include_directories('.'),
)

c_fndsa_dep = declare_dependency(
  sources: c_fndsa_sources,
  compile_args : compile_args,
  include_directories: include_directories('.'),
)

if meson.is_subproject()
  meson.override_dependency('c-fn-dsa', c_fndsa_dep)
  meson.override_dependency('c-fn-dsa-vrfy', c_fndsa_vrfy_dep)
  meson.override_dependency('c-fn-dsa-keygen', c_fndsa_keygen_dep)
  meson.override_dependency('c-fn-dsa-sign', c_fndsa_sign_dep)
endif
