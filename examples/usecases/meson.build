all_usecases = []

if machine_name == 'msp432e4'
  subdir('slot0')
else
  subdir('slot0')
endif
all_usecases += [{
  'name': usecase_name,
  'slot0': usecase_slot0,
  'slot1': usecase_slot1,
  'depends': usecase_depends,
  'has_slot0': usecase_has_slot0,
  'has_slot1': usecase_has_slot1,
}]

subdir('higher_version')
all_usecases += [{
  'name': usecase_name,
  'slot0': usecase_slot0,
  'slot1': usecase_slot1,
  'depends': usecase_depends,
  'has_slot0': usecase_has_slot0,
  'has_slot1': usecase_has_slot1,
}]

subdir('keygen_mix')
all_usecases += [{
  'name': usecase_name,
  'slot0': usecase_slot0,
  'slot1': usecase_slot1,
  'depends': usecase_depends,
  'has_slot0': usecase_has_slot0,
  'has_slot1': usecase_has_slot1,
}]

if machine_name != 'msp432e4'
subdir('boot_only')
all_usecases += [{
  'name': usecase_name,
  'slot0': usecase_slot0,
  'slot1': usecase_slot1,
  'depends': usecase_depends,
  'has_slot0': usecase_has_slot0,
  'has_slot1': usecase_has_slot1,
}]
endif

if machine_name == 'msp432e4'
  subdir('boot_only')
  all_usecases += [{
    'name': usecase_name,
    'slot0': usecase_slot0,
    'slot1': usecase_slot1,
    'depends': usecase_depends,
    'has_slot0': usecase_has_slot0,
    'has_slot1': usecase_has_slot1,
  }]
endif

if machine_name == 'renode-cm4' or machine_name == 'rx261' or machine_name == 'rx65n' or machine_name == 'msp432e4'
  subdir('ota')
  all_usecases += [{
    'name': usecase_name,
    'slot0': usecase_slot0,
    'slot1': usecase_slot1,
    'depends': usecase_depends,
    'has_slot0': usecase_has_slot0,
    'has_slot1': usecase_has_slot1,
  }]
endif
