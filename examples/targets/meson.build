target_option = get_option('target')
message('Building target: @0@'.format(target_option))

if target_option == 'all' or target_option == 'cm4'
  message('Adding target: cm4')
  subdir('cm4')
endif

if target_option == 'all' or target_option == 'ae_lpc11u35_mb'
  message('Adding target: ae_lpc11u35_mb')
  subdir('ae_lpc11u35_mb')
endif

# 将来のターゲット追加例:
# if target_option == 'all' or target_option == 'stm32f4'
#   message('Adding target: stm32f4')
#   subdir('stm32f4')
# endif
