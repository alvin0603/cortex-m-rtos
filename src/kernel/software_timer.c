#include "kernel/software_timer.h"
#include <stddef.h>

#define MAX_TIMERS 10
static softwareTimer* timer_list[MAX_TIMERS];
static uint32_t timer_count = 0;

void timer_create(softwareTimer *timer, uint32_t period, uint8_t is_periodic, timeCallback callback)
{
    timer_list[timer_count] = timer;
    timer_count++;
    timer->period = period;
    timer->remaining_ticks = period;
    timer->callback = callback;
    timer->is_active = 0;
    timer->is_periodic = is_periodic;
}

void timer_start(softwareTimer *timer)
{
    timer->remaining_ticks = timer->period; 
    timer->is_active = 1;
}

void timer_stop(softwareTimer *timer)
{
    timer->is_active = 0;
}

void timer_system_tick(void)
{
    for(uint32_t i = 0; i < timer_count; i++)
    {
        if(timer_list[i]->is_active)
        {
            timer_list[i]->remaining_ticks--;
            if(timer_list[i]->remaining_ticks == 0)
            {
                timer_list[i]->callback();
                if(timer_list[i]->is_periodic)
                    timer_list[i]->remaining_ticks = timer_list[i]->period;
            }
        }
    }
}