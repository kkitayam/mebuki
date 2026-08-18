# mebuki-svl-ecdsa-p256-sha256
mebuki SVL implementation using ECDSA P-256 and SHA-256.

- [BearSSL](https://bearssl.org/)

## Hardware acceleration (RX261 RSIP)

`-Daccel` controls RSIP usage for `-Dtarget=rx261`.

- `-Daccel=auto`: use RSIP when initialization succeeds; otherwise use software (BearSSL).
- `-Daccel=enabled`: require RSIP; configuration fails on non-RX261 targets.
- `-Daccel=disabled`: always use software.

## Public Keys Configuration

The SVL implementation requires public keys for signature verification. These keys are injected at compile time through a configurable header file.

### Using Public Keys

The SVL implementation includes the public keys from a header file specified by the `MBK_SVL_PUBLIC_KEYS_HEADER` macro:

- **Default header**: `public_keys.h` (must be on the include path)
- **Custom header**: Use `-DMBK_SVL_PUBLIC_KEYS_HEADER="custom_keys.h"` during compilation

### Template

The test directory contains [tests/public_keys.h](tests/public_keys.h) which serves as a template. To use SVL in your project:

1. Create a `public_keys.h` file in your project
2. Define the `public_keys` array following the format in the template:
   ```c
   static const uint8_t public_keys[MBK_SVL_NUM_KEY_GENERATIONS][MBK_SVL_PUBKEY_SIZE] = {
       /* Generation 0 */
       { /* 65-byte ECDSA P-256 uncompressed public key */ },
       /* Generation 1 */
       { /* 65-byte ECDSA P-256 uncompressed public key */ },
       // ... up to MBK_SVL_NUM_KEY_GENERATIONS
   };
   ```

3. Include your project's include path when building:
   ```bash
   gcc ... -I/path/to/your/project ...
   ```

Or override the header name:
   ```bash
   gcc ... -DMBK_SVL_PUBLIC_KEYS_HEADER="generated_keys.h" ...
   ```

### Key Generation

Public keys should be generated using mebuki-sign. The `public_keys.h` array is validated at compile time with static assertions to ensure:

- Array dimensions match configuration: `[MBK_SVL_NUM_KEY_GENERATIONS][MBK_SVL_PUBKEY_SIZE]`
- Each element is exactly `MBK_SVL_PUBKEY_SIZE` bytes (65 bytes for ECDSA P-256 uncompressed)
