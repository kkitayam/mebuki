subdir('fndsa')
subdir('ecdsa-p256-sha256')

accel_option = get_option('accel')
target_option = get_option('target')
rx261_target = target_option == 'rx261'

if accel_option == 'enabled' and not rx261_target
  error('accel=enabled requires target=rx261')
endif

use_rsip = (accel_option == 'enabled') or (accel_option == 'auto' and rx261_target)

if use_rsip
  subdir('ecdsa-p256-sha256-rsip')
  verification_ecdsa_selected_dep = verification_ecdsa_rsip_dep
else
  verification_ecdsa_selected_dep = verification_ecdsa_dep
endif

sign_algorithms = {
  'fndsa': 'fndsa512',
  'ecdsa-p256-sha256': 'ecdsa-p256-sha256',
}

verification_deps = {
  'fndsa': verification_fndsa_dep,
  'ecdsa-p256-sha256': verification_ecdsa_selected_dep,
}
