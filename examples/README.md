# mebuki-example
Example projects demonstrating mebuki secure boot library on virtual and physical MCU targets.

## Build

### Prerequisites

- Meson 1.10.0 or newer
- Ninja
- `arm-none-eabi` toolchain on `PATH`

### Configure

Renode Cortex-M4 target:

```powershell
meson setup builddir --cross-file cross/arm-none-eabi-gcc.ini -Dtarget=cm4
```

AE-LPC11U35-MB target:

```powershell
meson setup builddir --cross-file cross/arm-none-eabi-lpc11u35.ini -Dtarget=ae_lpc11u35_mb
```

To update an existing build directory, rerun the same command with `--reconfigure`.

### Compile

```powershell
meson compile -C builddir
```

### Outputs

- `cm4`: `boot.elf`, `boot.bin`, `app.elf`, `app.bin`
- `ae_lpc11u35_mb`: `boot.elf`, `boot.bin`, `app.elf`, `app.bin`

### App image variants

- `cm4_app_images`: `app.vX.kN.img` (`X=0..2`, `N=0..7`)
- `ae_app_images`: `app.vX.kN.img` (`X=0..2`, `N=0..7`)

### Optional targets

```powershell
meson compile -C builddir keygen
meson compile -C builddir cm4_app_images
meson compile -C builddir ae_app_images
meson compile -C builddir boot_size
meson compile -C builddir app_size
meson compile -C builddir renode_cm4_slot0
meson compile -C builddir renode_cm4_slot0_higher_version
meson compile -C builddir renode_cm4_slot0_keygen_mix
meson compile -C builddir renode_cm4_boot_only
```

`renode_cm4_slot0` は `boot.elf` と `app.v1.k7.img` / `app.v1.k0.img` を slot0 / slot1 に配置して GDB 3333 番で待機します。`renode_cm4_slot0_higher_version` と `renode_cm4_slot0_keygen_mix` は、別の security version / key generation 組み合わせで slot0 が選択されることを確認します。`renode_cm4_boot_only` は boot のみを読み込みます。

## Dependency Integration

This project resolves `mebuki`, `mebuki-svl-monocypher`, `cmsis-6`, and `nxp-driver-lpc11uxx` through Meson fallback subprojects.
