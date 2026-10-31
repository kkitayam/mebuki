# Machines

This directory contains the implementations of execution environments (Machines).

## Supported Machines

- `renode-cm4`: Reference environment running on Renode for Cortex-M4
- `renode-nucleo_h753zi`: Renode Nucleo H753ZI environment for Cortex-M7
- `rx261`: RX261 real board environment
- `rx65n`: RX65N real board environment
- `msp432e4`: MSP432E401Y real board environment

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

Each machine provides `scripts/run.py` as its execution entry point. By
default it deploys the requested image and then captures UART output. RX261
and RX65N support `--no-deploy` (or `-n`) to start firmware that is already
programmed. MSP432E4 uses the same option and issues a dslite system reset
(`-r 1`) before capturing UART. Renode always loads its images when it starts
and does not support `--no-deploy`.

## Renode Nucleo H753ZI

The H753ZI configuration reserves the first 128 KiB sector of each 1 MiB
flash bank for a copy of the boot program. The next sector in each bank stores
one boot history record; the remaining six sectors hold a firmware slot.
Both boot copies are loaded before execution.

The local flash controller extends Renode's STM32H7 controller. It applies the
programmed `SWAP_BANK_OPT` setting on reset, swaps the boot alias and bank
addresses, and maps bank-specific registers to the selected banks. It keeps
internal aliases at `0x08200000` and `0x08300000` so that Renode retains the
physical memory objects during remapping. These aliases are a simulator
implementation detail, not H753ZI hardware addresses. The platform maps the
boot flash at address zero instead of modeling ITCM.

This model covers the operations used by these examples. It does not model
the direct `OPTCR.SWAP_BANK` control, power interruption during option-byte
programming, flash wear, or persistence across separate Renode processes.
Renode's mapped flash memory does not enforce the controller lock or flash
bit-transition rules on direct CPU memory writes. The example HAL checks
erased state before programming, but the model cannot detect firmware that
bypasses it. Do not use this model to establish hardware flash safety or
power-loss guarantees.
