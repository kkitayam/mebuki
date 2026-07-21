#ifndef PUBLIC_KEYS_H
#define PUBLIC_KEYS_H

#include <assert.h>
#include <stdint.h>

#include "mebuki_svl.h"

extern uint8_t public_keys[MBK_SVL_NUM_KEY_GENERATIONS][MBK_SVL_PUBKEY_SIZE];

static_assert(
    sizeof(public_keys[0]) == MBK_SVL_PUBKEY_SIZE,
    "public_keys element size must be MBK_SVL_PUBKEY_SIZE"
);

#endif /* PUBLIC_KEYS_H */
