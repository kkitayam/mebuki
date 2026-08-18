# Machines

This directory contains the implementations of execution environments (Machines).

## Supported Machines

- `renode-cm4`: Reference environment running on Renode for Cortex-M4
- `rx261`: RX261 real board environment (boot only)

## Responsibilities of the Machine

- `hal/`: UART, flash, system, retarget
- `config/`: Memory map and mebuki configuration
- `startup_boot.s` / `startup_app.s`: Startup code
- `linker_boot.ld` / `linker_app.ld`: Linker script
- `renode/`: Environment configuration

Use Case is defined on the `examples/usecases/` side and is not retained on the Machine side.
