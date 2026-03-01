    .syntax unified
    .cpu cortex-m3
    .thumb

    .section .isr_vector, "a"
    .align 2
    .globl vector_table

vector_table:
    .word _stack_top            @ 0x00: Initial MSP (top of SRAM)
    .word Reset_Handler         @ 0x04: Reset
    .word Default_Handler       @ 0x08: NMI
    .word Default_Handler       @ 0x0C: HardFault
    .word Default_Handler       @ 0x10: MemManage
    .word Default_Handler       @ 0x14: BusFault
    .word Default_Handler       @ 0x18: UsageFault
    .word 0                     @ 0x1C: Reserved
    .word 0                     @ 0x20: Reserved
    .word 0                     @ 0x24: Reserved
    .word 0                     @ 0x28: Reserved
    .word Default_Handler       @ 0x2C: SVCall
    .word Default_Handler       @ 0x30: Debug Monitor
    .word 0                     @ 0x34: Reserved
    .word Default_Handler       @ 0x38: PendSV (context switching)
    .word Default_Handler       @ 0x3C: SysTick (system timer)

    .section .text
    .align 2

    .globl Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    @ Copy .data from FLASH to SRAM
    ldr r0, =_data_start
    ldr r1, =_data_end
    ldr r2, =_data_load

.copy_data:
    cmp r0, r1
    bge .zero_bss
    ldr r3, [r2], #4
    str r3, [r0], #4
    b   .copy_data

@ Zero out .bss in SRAM
.zero_bss:
    ldr r0, =_bss_start
    ldr r1, =_bss_end
    mov r2, #0

.zero_loop:
    cmp r0, r1
    bge .call_main
    str r2, [r0], #4
    b   .zero_loop

@ Jump to C
.call_main:
    bl  main
    b   .                       @ infinite loop if main returns

    .globl Default_Handler
    .type Default_Handler, %function
Default_Handler:
    b .

    .end
