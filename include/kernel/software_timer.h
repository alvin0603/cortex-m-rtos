#ifndef KERNEL_SOFTWARE_TIMER_H
#define KERNEL_SOFTWARE_TIMER_H

#include <stdint.h>

typedef void (*timeCallback)(void);
typedef struct
{
    uint32_t period;
    uint32_t remaining_ticks;
    timeCallback callback;
    uint8_t is_active;
    uint8_t is_periodic;
} softwareTimer;

void timer_create(softwareTimer *timer, uint32_t period, uint8_t is_periodic, timeCallback callback);
void timer_start(softwareTimer *timer);
void timer_stop(softwareTimer *timer);
void timer_system_tick(void);

#endif