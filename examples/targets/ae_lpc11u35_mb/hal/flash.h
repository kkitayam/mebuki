#ifndef FLASH_H
#define FLASH_H

#include <stddef.h>
#include <stdint.h>

/**
 * @file flash.h
 * @brief AE LPC11U35-MB の Flash HAL
 */

/**
 * @brief Flash 領域を読み出す。
 *
 * @param addr 読み出し開始アドレス
 * @param buf  書き込み先バッファ
 * @param len  読み出しサイズ [bytes]
 * @return 0: 成功, -1: 失敗
 */
int hal_flash_read(uint32_t addr, void *buf, size_t len);

/**
 * @brief Flash 領域へ書き込む。
 *
 * @param addr 書き込み開始アドレス
 * @param data 書き込み元データ
 * @param len  書き込みサイズ [bytes]
 * @return 0: 成功, -1: 失敗
 */
int hal_flash_write(uint32_t addr, const void *data, size_t len);

/**
 * @brief Flash セクターを消去する。
 *
 * @param addr セクター先頭アドレス
 * @return 0: 成功, -1: 失敗
 */
int hal_flash_erase_sector(uint32_t addr);

#endif /* FLASH_H */