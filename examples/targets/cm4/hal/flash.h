#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
#include <stddef.h>

/**
 * @file flash.h
 * @brief Flash メモリハードウェア抽象化層
 *
 * libmebuki (BFL, SML) が呼び出す Flash 操作インターフェース
 * - 読み出し: XiP (Execute in Place) 方式
 * - 書き込み: メモリマップドレジスタ操作
 * - 消去: セクター (4KB) 単位
 */

/**
 * @brief Flash メモリ読み出し
 *
 * @param addr   読み出し開始アドレス
 * @param buf    読み出しバッファ
 * @param len    読み出しサイズ (bytes)
 *
 * @return 成功時 0、失敗時 -1
 *
 * XiP 環境では memcpy 相当の実装
 */
int hal_flash_read(uint32_t addr, void* buf, size_t len);

/**
 * @brief Flash メモリ書き込み
 *
 * @param addr   書き込み開始アドレス
 * @param data   書き込みデータ
 * @param len    書き込みサイズ (bytes)
 *
 * @return 成功時 0、失敗時 -1
 *
 * 書き込み後、ベリファイ (読み戻し) を実施
 * Flash 特性: 0→1 の変更は不可（消去後に限定）
 */
int hal_flash_write(uint32_t addr, const void* data, size_t len);

/**
 * @brief Flash セクター消去
 *
 * @param addr   消去対象セクターの開始アドレス (4KB 境界)
 *
 * @return 成功時 0、失敗時 -1
 *
 * 指定セクター (4KB) を全て 0xFF (消去状態) にする
 */
int hal_flash_erase_sector(uint32_t addr);

#endif /* FLASH_H */
