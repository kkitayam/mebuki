# Getting Started

## Purpose

This document explains how to build and run mebuki.

This document is for first-time users.

## Prerequisites

Install these tools before you continue.

* Git
* Python 3
* [uv](https://docs.astral.sh/uv/
* [Meson](https://mesonbuild.com/)
* [Ninja](https://ninja-build.org/)
* A C compiler (Arm GCC for cross build, GCC or Clang for native build)

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

Windows:

```sh
.venv\Scripts\activate
```

Linux:

```sh
source .venv/bin/activate
```

## Install mebuki-sign

Install the image signing tool.

The source code is available from [mebuki-sign](https://github.com/kkitayam/mebuki-sign).

Install the latest version from the Git repository.

```sh
uv pip install git+https://github.com/kkitayam/mebuki-sign.git
```

Verify the installation.

```sh
mebuki-sign --help
```

## Run an Example

Configure a cross build.

```sh
uv run meson setup builddir-cross --cross-file cross/arm-none-eabi-gcc.ini
```

Build and run the `run_slot0` example.

```sh
uv run meson compile -C builddir-cross run_slot0
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

The values in the output can change.

The final line, `hello world`, confirms that the example ran successfully.

The repository provides these example targets.

| Target               | Description                                                                |
| -------------------- | -------------------------------------------------------------------------- |
| `run_slot0`          | Boot a valid image from slot 0.                                            |
| `run_higher_version` | Boot the bootable image that has the highest version.                      |
| `run_keygen_mix`     | Boot an image after signature verification with different key generations. |
| `run_boot_only`      | Start the bootloader without an application image.                         |

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

## Use mebuki in Your Project

Add `mebuki_dep` to your Meson project.

```meson
dependencies: [mebuki_dep]
```

Include only the public API.

```c
#include <mebuki.h>
```

Do not include internal headers.

SVL implementations use `mebuki_svl_dep`.

Application code must not use this dependency.
