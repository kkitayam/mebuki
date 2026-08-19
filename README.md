# mebuki

A lightweight secure boot library for resource-constrained MCUs.

Mebuki provides the core functions that a secure boot process requires.

* Firmware authentication
* Rollback protection
* Verification key management
* Persistent security state management

Mebuki separates secure boot logic from cryptographic implementations. This design lets you change the signature algorithm without changing the boot flow.

Mebuki does not provide firmware download, image storage, or flash programming. Your application controls these functions.

## Goals

* Public-key secure boot
* Post-quantum ready
* Power-loss resilient
* Portable
* Small footprint

## Features

### Firmware Authentication

Mebuki verifies firmware authenticity and integrity with digital signatures.

Select the verification algorithm at build time.

Supported algorithms:

* ECDSA-P256-SHA256
* FN-DSA-512 (FIPS 206)

### Rollback Protection

Mebuki prevents firmware rollback.

Each firmware image contains a Security Version.

Mebuki rejects firmware that uses an older Security Version than the stored value.

### Verification Key Rotation

Mebuki supports forward-only verification key rotation.

The device stores multiple verification keys.

When verification succeeds with a newer key, Mebuki updates the active key automatically.

Mebuki never enables an older key again.

### Persistent Security State

Mebuki stores the Security Version and the active verification key in redundant boot history records.

This design protects the security state from unexpected power loss.

### Optional Slot Swap

Mebuki provides an optional slot swap mechanism.

If power fails during a slot swap, Mebuki resumes the operation after reset.

## Quick Start

See [docs/getting_started.md](docs/getting_started.md) for the complete setup procedure.

Typical commands:

```sh
git clone https://github.com/kkitayam/mebuki.git
cd mebuki

uv venv
. .venv/bin/activate        # Linux
# .venv\Scripts\activate    # Windows

uv pip install git+https://github.com/kkitayam/mebuki-sign.git

uv run meson setup builddir \
    --cross-file cross/arm-none-eabi-gcc.ini \
    -Dtarget=renode-cm4 \
    -Dsvl=fndsa

uv run meson compile -C builddir run_slot0
```

## Repository Layout

```text
mebuki/
├── mebuki/          Core secure boot library
│   ├── include/
│   └── src/
├── verification/    Verification algorithm integration
│   ├── ecdsa-p256-sha256/
│   └── fndsa/
├── machines/        Target machine support
├── cross/           Meson cross files
├── examples/        Reference applications
├── tests/           Unit tests
├── docs/            Documentation
├── vendor/          Bundled third-party source code
└── subprojects/     External dependencies
```

The [verification/](verification/) directory contains only the integration layer.

Third-party cryptographic libraries remain separate.

## Requirements

Mebuki requires:

* A resource-constrained MCU
* A C23 compiler
* A little-endian architecture
* Execute-in-Place (XiP) NOR Flash
* Platform-specific Flash read, write, and erase functions

## Out of Scope

Mebuki does not provide:

* Firmware download
* Communication protocols
* Firmware image programming
* Device-specific boot initialization

These functions remain under application control.

## Cryptographic Libraries

Mebuki delegates cryptographic operations to external libraries.

| Verification Algorithm | Library |
|-------------------------|---------|
| ECDSA-P256-SHA256 | [BearSSL](https://bearssl.org/) |
| FN-DSA-512 | [c-fn-dsa](https://github.com/pornin/c-fn-dsa) |

This separation keeps the secure boot framework small and easy to maintain.

## Project Status

Mebuki is a hobby open-source project.

The project is intended for learning, experimentation, and evaluation.

Do not use Mebuki as the primary secure boot solution for production products.

For production systems, use an established project such as [MCUboot](https://github.com/mcu-tools/mcuboot).

## License

See `LICENSE`.