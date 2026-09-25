# mebuki examples

`examples/` has only examples of library usage.

## Layout

```text
examples/
├── boot/
├── app/
├── app_ota/
└── usecases/
    ├── slot0/
    ├── higher_version/
    ├── keygen_mix/
    └── boot_only/
    └── ota/
```

- `boot/`, `app/`: Reference implementations of boot and application software
- `app_ota/`: YMODEM OTA receiver that writes a signed image to slot1
- `usecases/`: Use case definitions (independent `meson.build`)

Runtime environment (startup, linker, HAL, Renode) is separated into `machines/`.

## Use Cases

Use cases define which application images load into slot0 and slot1.
Each use case declares only the image selection data.
Deploy and run logic lives in machine definitions.

### Supported Machines

- **renode-cm4**: All use cases supported. Run targets start the Renode simulator.
- **rx261** and **rx65n**: All use cases supported. Run targets deploy images
  via rfp-cli and run them on hardware through the configured serial port.
- **msp432e4**: Use cases supported through dslite and the configured serial
  port.

Every machine uses `machines/<machine>/scripts/run.py`. A use-case `run_*`
target deploys images before starting the application. The boot `run` target
uses `--no-deploy` and starts firmware that is already programmed.

## Build

### Prerequisites

- Meson 1.10.0 or newer
- Ninja
- `arm-none-eabi` toolchain on `PATH` (for renode-cm4)
- `rx-elf-gcc` toolchain on `PATH` (for RX261 and RX65N)

### Configure

For Renode (CM4):

```powershell
meson setup builddir --cross-file cross/arm-none-eabi-gcc.ini -Dtarget=renode-cm4
```

For RX261 or RX65N hardware:

```powershell
meson setup builddir --cross-file cross/rx-elf-gcc.ini -Dtarget=rx261 -Dserial_port=<port>
```

Use `-Dtarget=rx65n` and `cross/rx65n-elf-gcc.ini` for RX65N:

```powershell
meson setup builddir-rx65n --cross-file cross/rx65n-elf-gcc.ini -Dtarget=rx65n -Dserial_port=<port>
```

### Compile

```powershell
meson compile -C builddir
```

Generates `boot.elf`, `app.vX.kY.img` for all use cases.
`vX` is the version number, `kY` is the key generation number.

For Renode, run the OTA demonstration with `meson compile -C builddir run_ota`.
It loads `app_ota` in slot0, sends the signed version 2 application image over
the Renode UART socket, and verifies boot-driven promotion from slot1 to slot0.

For RX261 and RX65N, run the OTA demonstration with `meson compile -C
builddir run_ota`. The run target deploys `app_ota`, waits for its UART
banner, and sends the signed version 2 application image over the configured
serial port. RX65N writes the image to the second 1 MiB flash bank and uses
the hardware bank-swap feature before booting the updated application.

```powershell
uv run meson compile -C builddir run_ota
```

For MSP432E4, configure with the Arm cross file and the board serial port:

```powershell
uv run meson setup builddir-msp432e4 --cross-file cross/arm-none-eabi-gcc.ini `
    -Dtarget=msp432e4 -Dserial_port=COM5
uv run meson compile -C builddir-msp432e4 run_slot0
```

The RX261 receiver programs code flash in 8-byte units. RX65N programs in
128-byte units. Both receivers pad only the final programming unit with
erased (`0xFF`) bytes, preserving the signed image contents.
