# mebuki-svl-ecdsa-p256-sha256-rsip
mebuki SVL implementation using ECDSA P-256 and SHA-256 via RX261 RSIP hardware engine.

Requires the RX261 RSIP-E11A peripheral (`vendor/rx261/rsip_ecdsa_p256_sha256.*`).

This implementation is selected automatically when `-Dtarget=rx261 -Dsvl=ecdsa-p256-sha256`
is used with `-Daccel=auto` (default) or `-Daccel=enabled`.

To force software (BearSSL) on RX261, use `-Daccel=disabled`.

## accel option

| `-Daccel` | `target=rx261` | Implementation compiled |
|---|---|---|
| `auto` (default) | yes | RSIP (this directory) |
| `auto` (default) | no  | BearSSL (`ecdsa-p256-sha256/`) |
| `enabled` | yes | RSIP (this directory) |
| `enabled` | no  | configure error |
| `disabled` | any | BearSSL (`ecdsa-p256-sha256/`) |

## Public Keys Configuration

The SVL implementation requires public keys for signature verification. These keys are injected at compile time through a configurable header file.

### Using Public Keys

The SVL implementation includes the public keys from a header file specified by the `MBK_SVL_PUBLIC_KEYS_HEADER` macro:

- **Default header**: `public_keys.h` (must be on the include path)
- **Custom header**: Use `-DMBK_SVL_PUBLIC_KEYS_HEADER="custom_keys.h"` during compilation
