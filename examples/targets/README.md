# Targets

このディレクトリには、Meson ベースのターゲット実装が含まれます。

## 利用可能なターゲット

- `cm4`: Renode 上で動く Cortex-M4 参照ターゲット
- `ae_lpc11u35_mb`: Akizuki AE-LPC11U35-MB 実機ターゲット

## 共通レイアウト

各ターゲットは次の要素を持ちます。

- `hal/`: UART, flash, system, retarget
- `config/`: メモリマップと mebuki 設定
- `startup_boot.s` / `startup_app.s`: 起動コード
- `linker_boot.ld` / `linker_app.ld`: リンカスクリプト
- `meson.build`: ターゲット定義

`cm4` には Renode 起動用の Meson run target (`renode_cm4_slot0`, `renode_cm4_boot_only`) も含まれます。
`cm4` には追加で `renode_cm4_slot0_higher_version` と `renode_cm4_slot0_keygen_mix` があり、`cm4_app_images` で `app.vX.kN.img` をまとめて生成できます。

## AE-LPC11U35-MB

このターゲットは CMSIS Core と LPCOpen を Meson subproject として参照します。
Flash レイアウトは 4KB sector erase と 256B page program を前提にしています。
