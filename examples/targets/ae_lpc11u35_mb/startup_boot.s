.syntax unified
.cpu cortex-m0
.thumb

.global __Vectors
.global ResetISR
.global Default_Handler
.global NMI_Handler
.global HardFault_Handler
.global SVCall_Handler
.global PendSV_Handler
.global SysTick_Handler

.extern __StackTop
.extern __data_load_start
.extern __data_start
.extern __data_end
.extern __bss_start
.extern __bss_end
.extern halt
.extern main

.section .isr_vector,"a",%progbits
.align 2
__Vectors:
  .word __StackTop
  .word ResetISR + 1
  .word NMI_Handler + 1
  .word HardFault_Handler + 1
  .word 0
  .word 0
  .word 0
  .word __valid_user_code_checksum
  .word 0
  .word 0
  .word 0
  .word SVCall_Handler + 1
  .word 0
  .word 0
  .word PendSV_Handler + 1
  .word SysTick_Handler + 1
  .rept 32
  .word Default_Handler + 1
  .endr

.text
.align 2

.thumb_func
ResetISR:
  ldr r0, =__data_load_start
  ldr r1, =__data_start
  ldr r2, =__data_end

1:
  cmp r1, r2
  bcs 2f
  ldr r3, [r0]
  str r3, [r1]
  adds r0, #4
  adds r1, #4
  b 1b

2:
  ldr r1, =__bss_start
  ldr r2, =__bss_end
  movs r3, #0

3:
  cmp r1, r2
  bcs 4f
  str r3, [r1]
  adds r1, #4
  b 3b

4:
  bl main
  bl halt

.thumb_func
Default_Handler:
  b .

.weak NMI_Handler
.thumb_set NMI_Handler, Default_Handler

.weak HardFault_Handler
.thumb_set HardFault_Handler, Default_Handler

.weak SVCall_Handler
.thumb_set SVCall_Handler, Default_Handler

.weak PendSV_Handler
.thumb_set PendSV_Handler, Default_Handler

.weak SysTick_Handler
.thumb_set SysTick_Handler, Default_Handler
