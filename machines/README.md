# Machines

This directory contains the implementations of execution environments (Machines).

## Supported Machines

- `renode-cm4`: Reference environment running on Renode for Cortex-M4
- `rx261`: RX261 real board environment
- `rx65n`: RX65N real board environment

## Responsibilities of the Machine

- `hal/`: UART, flash, system, retarget
- `config/`: Memory map and mebuki configuration
- `startup_boot.c` / `startup_app.c`: Startup code
- `linker_boot.ld` / `linker_app.ld`: Linker script
- `renode/`: Environment configuration

The RX261 and RX65N machines use the Renesas RX toolchain and rfp-cli for
hardware deployment. RX65N uses the RX65N Type 4 flash driver and hardware
bank swapping for its two 1 MiB code-flash banks; boot software is deployed
to both banks.

Use Case is defined on the `examples/usecases/` side and is not retained on the Machine side.
