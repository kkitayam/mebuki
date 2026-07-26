# mebuki

A lightweight secure boot library for resource-constrained MCUs with native support for **Post-Quantum Cryptography (PQC)**.

Mebuki focuses exclusively on the core responsibilities of secure boot -- firmware authentication, rollback protection, verification key management, and persistent security state management. Its modular architecture separates secure boot logic from cryptographic implementations, allowing signature algorithms to evolve independently without changing the boot flow.

Unlike complete firmware update frameworks, Mebuki intentionally leaves firmware download, image storage, and flash programming under application control, providing only the secure boot functionality required to verify and activate trusted firmware.

## Goals

- ✓ Public-key Secure Boot
- ✓ Post-Quantum Ready
- ✓ Power-loss Resilient
- ✓ Portable
- ✓ Small Footprint

## Features

### Security

#### Firmware Authentication

Verifies firmware authenticity and integrity using digital signatures.

Supports **ECDSA-P256-SHA256** and the post-quantum signature scheme **FN-DSA-512** (FIPS 206). The verification algorithm is selected at build time to balance security, performance, and code size.

#### Rollback Protection

Prevents firmware rollback using a dedicated **Security Version** field stored in the image header.

#### Pre-provisioned Verification Key Rotation

Supports forward-only verification key rotation using a pre-provisioned public key list.

When firmware signed with the next (or any subsequent) verification key is successfully verified, Mebuki automatically advances the active verification key. Previously accepted keys are permanently retired after advancement.

### Reliability

#### Power-loss Resilient Security State Management

Maintains the integrity of the persistent security state -- including the Security Version and active verification key -- across unexpected power loss using redundant boot history records.

### Update Support

#### Optional Resumable Slot Swap

Provides an optional power-loss resilient slot swap mechanism that automatically resumes and completes an interrupted slot swap after reset.

## Quick Start

Build the project using Meson.

```sh
git clone <repository>
cd mebuki

meson setup build
meson compile -C build
```

The reference example requires a firmware image signed with **mebuki-sign**.

Generate a signed firmware image using `mebuki-sign`, then run the reference example.

For the complete workflow -- including key generation, firmware signing, image creation, and execution -- see the **Getting Started** guide in `docs/`.

## Repository Layout

```text
mebuki/
├── mebuki/             Core secure boot library
│   ├── include/
│   └── src/
│
├── verification/       Verification algorithm implementations
│   ├── ecdsa-p256-sha256/
│   └── fndsa/
│
├── machines/           Target machine implementations (startup/linker/HAL/runner)
├── cross/              Meson cross files
├── examples/           Reference boot/app and use case definitions
├── tests/              Unit tests
├── docs/               Documentation
├── vendor/             Bundled third-party source code
└── subprojects/        External dependencies managed by Meson Wrap
```

The `verification/` directory contains only the integration layer between Mebuki and each verification algorithm. Third-party cryptographic libraries are managed separately under `subprojects/`.

## Supported Platforms

Mebuki is designed for resource-constrained embedded systems.

### Requirements

- Resource-constrained MCU
- C99 compiler
- Little-endian architecture
- Execute-in-Place (XiP) NOR Flash
- Platform-specific Flash read/write/erase driver

### Out of Scope

Mebuki intentionally does **not** provide:

- Firmware download
- Communication protocols
- Firmware image programming
- Device-specific boot initialization

These responsibilities remain under application control.

## Cryptographic Libraries

Mebuki delegates cryptographic operations to dedicated external libraries rather than implementing signature algorithms itself.

| Verification Algorithm | Library |
| ---------------------- | ------- |
| **ECDSA-P256-SHA256** | BearSSL by Thomas Pornin |
| **FN-DSA-512** | c-fn-dsa by Thomas Pornin |

This separation keeps the secure boot framework compact, maintainable, and easy to audit while allowing cryptographic implementations to evolve independently.

## Documentation

The following documents provide more detailed information.

- Getting Started
- Architecture Overview
- Porting Guide
- Image Format Specification
- API Reference

## Project Status

Mebuki is a personal open-source project developed with an emphasis on simplicity, portability, and modern cryptography.

Although designed with production-quality engineering principles, it has not yet reached the maturity or ecosystem of long-established secure boot projects.

For commercial products or long-term maintained platforms, consider evaluating established solutions such as [MCUboot](https://github.com/mcu-tools/mcuboot) alongside Mebuki to determine the best fit for your requirements.

## License

See the `LICENSE` file for licensing information.
