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

## Use Cases

Use cases define which application images load into slot0 and slot1.
Each use case declares only the image selection data.
Deploy and run logic lives in machine definitions.

### Supported Machines

- **renode-cm4**: All use cases supported. Run targets start Renode simulator.
- **rx261**: All use cases supported. Run targets deploy images via rfp-cli and run on hardware.

## Build

### Prerequisites

- Meson 1.10.0 or newer
- Ninja
- `arm-none-eabi` toolchain on `PATH` (for renode-cm4)
- `rx-elf-gcc` toolchain on `PATH` (for rx261)

### Configure

For Renode (CM4):

```powershell
meson setup builddir --cross-file cross/arm-none-eabi-gcc.ini -Dtarget=renode-cm4
```

For RX261 hardware:

```powershell
meson setup builddir --cross-file cross/rx-elf-gcc.ini -Dtarget=rx261 -Dserial_port=<port>
```

### Compile

```powershell
meson compile -C builddir
```

Generates `boot.elf`, `app.vX.kY.img` for all use cases.
`vX` is the version number, `kY` is the key generation number.
