# Machines

このディレクトリには、実行環境（Machine）実装を配置します。

## 利用可能な Machine

- `renode-cm4`: Renode 上で動作する Cortex-M4 参照環境

## 含める責務

- `hal/`: UART, flash, system, retarget
- `config/`: メモリマップと mebuki 設定
- `startup_boot.s` / `startup_app.s`: 起動コード
- `linker_boot.ld` / `linker_app.ld`: リンカスクリプト
- `renode/`: 実行環境設定

Use Case は `examples/usecases/` 側で定義し、Machine 側には保持しません。
