# mebuki examples

`examples/` はライブラリ利用例のみを保持します。

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

- `boot/`, `app/`: 参照実装（各 1 セット）
- `usecases/`: 実行シナリオ定義（独立した `meson.build`）

実行環境（startup, linker, HAL, Renode）は `machines/` に分離されています。

## Build

### Prerequisites

- Meson 1.10.0 or newer
- Ninja
- `arm-none-eabi` toolchain on `PATH`

### Configure

```powershell
meson setup builddir --cross-file cross/arm-none-eabi-gcc.ini -Dtarget=renode-cm4
```

既存 builddir の更新は `--reconfigure` を指定してください。

### Compile

```powershell
meson compile -C builddir
```

デフォルトビルドは全 Use Case を生成します。

個別の Use Case:

```powershell
meson compile -C builddir boot_only
meson compile -C builddir slot0
meson compile -C builddir higher_version
meson compile -C builddir keygen_mix
meson compile -C builddir usecases
```

### Development targets

```powershell
meson compile -C builddir boot
meson compile -C builddir app
meson compile -C builddir keygen
meson compile -C builddir renode_cm4_app_images
meson compile -C builddir boot_size
meson compile -C builddir app_size
```

### Run targets

```powershell
meson compile -C builddir renode_cm4_slot0
meson compile -C builddir renode_cm4_slot0_higher_version
meson compile -C builddir renode_cm4_slot0_keygen_mix
meson compile -C builddir renode_cm4_boot_only
```
