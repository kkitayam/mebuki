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
    -Daccel=disabled
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

To restart the bootloader without deploying an image, run the `run` target.
The target uses the image already stored on the machine and captures UART
output. The former `deploy` target is no longer provided.

## RX261 Use Cases

RX261 hardware support is available for all use cases.

Verify the serial port is connected first.

Configure the build:

```bash
uv run meson setup builddir-rx261 \
    --cross-file cross/rx-elf-gcc.ini \
    -Dtarget=rx261 \
    -Dsvl=ecdsa-p256-sha256 \
    -Dserial_port=COM3
```

Run a use case by name.
Available targets are `run_slot0`, `run_higher_version`, `run_keygen_mix`, and `run_boot_only`.

Example:

```bash
uv run meson compile -C builddir-rx261 run_slot0
```

This command:
1. Deploys the boot image and selected app images to flash
2. Starts the application via rfp-cli
3. Captures and displays UART logs

The UART console displays output similar to the Renode examples above.

The runner deploys images by default. Use `-n` or `--no-deploy` with the
machine `run.py` script when the images are already programmed.

## RX65N Use Cases

RX65N supports `run_slot0`, `run_higher_version`, `run_keygen_mix`, and
`run_boot_only`. Configure the target with the RX65N cross file and the
connected serial port.

```bash
uv run meson setup builddir-rx65n \
    --cross-file cross/rx65n-elf-gcc.ini \
    -Dtarget=rx65n \
    -Dserial_port=COM3
uv run meson compile -C builddir-rx65n run_higher_version
```

The default `rx65n_data_location=dataflash` stores the boot flash-level data
in Data Flash. To store it in the two Code Flash sectors immediately before
the boot image and its bank-swap pair, configure the build with
`-Drx65n_data_location=codeflash`.

The RX65N runner deploys the boot image to both flash banks and places the
selected signed application images at the slot0 and slot1 bank addresses.
`run_higher_version` verifies bank swap after selecting the higher security
version, and `run_keygen_mix` verifies images signed with different key
generations. OTA remains unsupported for RX65N.

## MSP432E4 Use Cases

MSP432E4 supports the use-case targets with the TI `dslite` tool and the
configured `MSP432E401Y.ccxml` file. Configure the build with the serial port
used for UART output (the default is `COM5`), then run a target such as
`run_slot0`. The target deploys the firmware, starts it, and captures UART.

The boot-only `run` target skips deployment and captures UART from the
firmware already programmed on the board.

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
