# Test-only compile definitions
test_c_args = ['-DUNIT_TEST']

# Common includes and sources
common_includes = [
  mbk_svl_includes,
  include_directories('../mocks'),
]
common_sources = [
  '../mocks/mock_hal_flash.c',
  '../mocks/mock_svl.c',
]

# Test definitions (name, source)
tests = [
  {
    'name' : 'test_bel',
    'src'  : 'test_bel.c',
  },
  {
    'name' : 'test_bfl',
    'src'  : 'test_bfl.c',
  },
  {
    'name' : 'test_sml',
    'src'  : 'test_sml.c',
  },
  {
    'name' : 'test_svl',
    'src'  : 'test_svl.c',
  },
  {
    'name' : 'test_bsl',
    'src'  : 'test_bsl.c',
  },
]

foreach t : tests
  exe = executable(t['name'], [t['src']] + common_sources,
    include_directories : common_includes,
    dependencies: [mebuki_dep, unity_dep],
    c_args : test_c_args,
    build_by_default : false
  )
  test(t['name'], exe, suite : 'mebuki')
endforeach

# Run the taneue test suite under each wear optimization policy.
foreach policy : ['TANEUE_WEAR_OPT_PRIMARY', 'TANEUE_WEAR_OPT_BALANCED']
  name = 'test_taneue_' + policy.to_lower().replace('taneue_wear_opt_', '')
  exe = executable(name, ['test_taneue.c'] + common_sources,
    include_directories : common_includes,
    dependencies : [mebuki_dep, unity_dep],
    c_args : test_c_args + ['-DTANEUE_WEAR_OPT=' + policy],
    build_by_default : false
  )
  test(name, exe, suite : 'mebuki')
endforeach
