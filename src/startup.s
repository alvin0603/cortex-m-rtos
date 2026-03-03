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
    .word PendSV_Handler        @ 0x38: PendSV (context switching)
    .word SysTick_Handler       @ 0x3C: SysTick (system timer)

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
    b   .                       @ infinite loop if main return

    .globl PendSV_Handler
    .type PendSV_Handler, %function
PendSV_Handler:
    @ Save old task's R4-R11
    mrs r0, psp                 @ r0 = current PSP (old task's stack top)
    stmfd r0!, {r4-r11}         @ push R4-R11 onto old task's stack

    @ Save updated SP back to old task's TCB
    ldr r1, =current_task       @ r1 = &current_task
    ldr r2, [r1]                @ r2 = current_task (pointer to old TCB)
    str r0, [r2]                @ current_task->stack_pointer = r0

    @ Switch to new task
    ldr r3, =next_task          @ r3 = &next_task
    ldr r0, [r3]                @ r0 = next_task (pointer to new TCB)
    str r0, [r1]                @ current_task = next_task

    @ Restore new task's R4-R11
    ldr r0, [r0]                @ r0 = next_task->stack_pointer
    ldmfd r0!, {r4-r11}         @ pop R4-R11 from new task's stack

    @ Update PSP and return
    msr psp, r0                 @ PSP = new task's stack pointer
    bx lr                       @ return (hardware automatically restores R0-R3, R12, LR, PC, xPSR)

    .globl Default_Handler
    .type Default_Handler, %function
Default_Handler:
    b .

    .end
