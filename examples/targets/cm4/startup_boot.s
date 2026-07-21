/**
 * @file startup.s
 * @brief Cortex-M4 スタートアップコード
 *
 * - ベクタテーブル定義
 * - スタックポインタ初期化
 * - main() 関数へジャンプ
 */

    .syntax unified
    .cpu cortex-m4
    .thumb

    .section .isr_vector, "a"
    .align 2
    .globl __isr_vector
__isr_vector:
    /* スタックポインタ初期値 */
    .long   0x20020000                          /* SRAM end (128KB) */

    /* リセットハンドラ */
    .long   reset_handler
    
    /* Cortex-M4 ハンドラ (使用しない) */
    .long   0                                   /* NMI */
    .long   0                                   /* HardFault */
    .long   0                                   /* MemManage */
    .long   0                                   /* BusFault */
    .long   0                                   /* UsageFault */
    .long   0                                   /* Reserved */
    .long   0                                   /* Reserved */
    .long   0                                   /* Reserved */
    .long   0                                   /* Reserved */
    .long   0                                   /* SVCall */
    .long   0                                   /* Reserved */
    .long   0                                   /* Reserved */
    .long   0                                   /* PendSV */
    .long   0                                   /* SysTick */

    /* IRQ ハンドラ (Renode では使用しない) */
    .long   0                                   /* IRQ0-239 用スペース */

    .text
    .align 2
    .globl reset_handler
    .thumb_func
reset_handler:
    /* メインプログラムへジャンプ */
    bl      main

    /* main() が戻ってきた場合 */
    b       .

    .end
