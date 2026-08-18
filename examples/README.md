# mebuki examples

`examples/` has only examples of library usage.

## Layout

```text
examples/
├── boot/
├── app/
└── usecases/
    ├── slot0/
    ├── higher_version/
    ├── keygen_mix/
    └── boot_only/
```

- `boot/`, `app/`: Reference implementations of boot and application software
- `usecases/`: Use case definitions (independent `meson.build`)

Runtime environment (startup, linker, HAL, Renode) is separated into `machines/`.

For `-Dtarget=rx261`, only `boot/` is built.

## Build

### Prerequisites

- Meson 1.10.0 or newer
- Ninja
- `arm-none-eabi` toolchain on `PATH`

### Configure

```powershell
meson setup builddir --cross-file cross/arm-none-eabi-gcc.ini -Dtarget=renode-cm4
```

### Compile

```powershell
meson compile -C builddir
```

Generate `boot.elf`, `app.vX.kY.img` for all Use Cases. `vX` is the version number, `kY` is the key generation number.
