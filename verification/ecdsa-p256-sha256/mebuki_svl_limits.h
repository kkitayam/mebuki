/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#ifndef MEBUKI_SVL_LIMITS_H
#define MEBUKI_SVL_LIMITS_H

/* ECDSA P-256 sizes */
#define MBK_SVL_PUBKEY_SIZE      65U   /* uncompressed P-256 public key: 0x04 || 32-byte x || 32-byte y */
#define MBK_SVL_SIGNATURE_SIZE   64U   /* raw ECDSA P-256 signature: 32-byte r || 32-byte s */
#define MBK_SVL_HASH_SIZE        32U   /* SHA-256 digest size */

#endif /* MEBUKI_SVL_LIMITS_H */
