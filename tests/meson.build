unity_includes = include_directories('unity')
unity_sources = files('unity/unity.c')

unity_dep = declare_dependency(
  sources: unity_sources,
  include_directories: unity_includes,
)

subdir('mebuki')
subdir('verification/fndsa')
subdir('verification/ecdsa-p256-sha256')
