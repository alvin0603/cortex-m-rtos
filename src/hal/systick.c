#include "hal/systick.h"

#define SYSTICK_CSR   (*((volatile uint32_t *)0xE000E010))
#define SYSTICK_RVR   (*((volatile uint32_t *)0xE000E014))
#define SYSTICK_CVR   (*((volatile uint32_t *)0xE000E018))

void systick_init(uint32_t ticks)
{
    SYSTICK_RVR = ticks - 1;
    SYSTICK_CVR = 0;
    SYSTICK_CSR = 0x07; // Bit 0: Enable, Bit 1: Tick Int (Trigger interrupt), Bit 2: Clock Source (CPU or External)
}
