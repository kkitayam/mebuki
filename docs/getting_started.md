# Getting Started

## Purpose

This document explains how to build and run Mebuki.

This document is for first-time users.

## Prerequisites

Install these tools before you continue.

* Git
* Python 3
* [uv](https://docs.astral.sh/uv/)
* [Meson](https://mesonbuild.com/)
* [Ninja](https://ninja-build.org/)
* A C compiler
  * Arm GNU Toolchain for cross builds
  * GCC or Clang for native builds

Use a compiler that supports C23.

## Get the Source Code

Clone the repository.

```sh
git clone https://github.com/kkitayam/mebuki.git
cd mebuki
```

## Create a Python Environment

Create a virtual environment.

```sh
uv venv
```

Activate the environment.

### Windows

```sh
.venv\Scripts\activate
```

### Linux

```sh
source .venv/bin/activate
```

## Install mebuki-sign

Mebuki uses **mebuki-sign** to generate signed firmware images.

The project is available at:

https://github.com/kkitayam/mebuki-sign

Install the latest version from the Git repository.

```sh
uv pip install git+https://github.com/kkitayam/mebuki-sign.git
```

Verify the installation.

```sh
mebuki-sign --help
```

## Run a Reference Example

Configure a cross build.

```sh
uv run meson setup builddir \
    --cross-file cross/arm-none-eabi-gcc.ini \
    -Dtarget=renode-cm4 \
    -Dsvl=fndsa \
    -Daccel=auto
```

Build and run the first example.

```sh
uv run meson compile -C builddir run_slot0
```

Renode starts automatically.

The UART console displays output similar to this.

```text
 Software (mebuki)
==================================================
swap slots if needed...
Initializing mebuki...
Finding bootable slot...
Bootable slot found!
  Slot ID: 0
  Security Version: 0x1
  Key Generation: 0x0
  Software Size: 0xAB8
  Entry Point: 0x00020008
Booting slot 0...
hello world
```

The values can differ.

The final line, `hello world`, confirms that the application started successfully.

The repository provides these example targets.

| Target | Description |
|---------|-------------|
| `run_slot0` | Boot a valid image from slot 0. |
| `run_higher_version` | Boot the image that has the highest Security Version. |
| `run_keygen_mix` | Verify images that use different verification key generations. |
| `run_boot_only` | Start only the bootloader. Do not load an application image. |

Build another example by replacing the target name.

Example:

```sh
uv run meson compile -C builddir run_higher_version
```

## RX261 (Boot only)

RX261 support is for boot software only.

```bash
uv run meson setup builddir-rx261 \
    --cross-file cross/rx-elf-gcc.ini \
    -Dtarget=rx261 \
    -Dsvl=ecdsa-p256-sha256 \
    -Daccel=auto \
    -Dserial_port=COM3
```

```bash
uv run meson compile -C builddir-rx261 deploy
uv run meson compile -C builddir-rx261 run
```

## Run the Unit Tests

Configure a native build.

Do not specify a cross file.

```sh
uv run meson setup builddir-native
```

Run all unit tests.

```sh
uv run meson test -C builddir-native
```

The unit tests run on the host computer.

## Use Mebuki in Your Project

Add `mebuki_dep` to your Meson target.

```meson
dependencies : [mebuki_dep]
```

Include the public API.

```c
#include <mebuki.h>
```

Do not include internal headers.

SVL implementations use `mebuki_svl_dep`.

Application code must not use this dependency.
