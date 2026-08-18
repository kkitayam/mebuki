target_option = get_option('target')
message('Building machine: @0@'.format(target_option))

objcopy = find_program('objcopy')
size = find_program('size')

if target_option == 'all' or target_option == 'renode-cm4' or target_option == 'cm4'
  if target_option == 'cm4'
    warning('target=cm4 is deprecated. use target=renode-cm4')
  endif
  subdir('renode-cm4')
elif target_option == 'rx261'
  subdir('rx261')
endif
