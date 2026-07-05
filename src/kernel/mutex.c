#include "kernel/mutex.h"
#include "kernel/scheduler.h"
#include <stddef.h>
#include <stdint.h>

void mutex_init(Mutex *mtx)
{
    mtx->locked = 0;
    mtx->owner = NULL;
    mtx->waiting_count = 0;
}
void mutex_lock(Mutex *mtx)
{
    while(1)
    {
        critical_enter();
        if(mtx->locked == 0)
        {
            mtx->locked = 1;
            mtx->owner = current_task;
            critical_exit();
            return ;
        }
        else 
        {
            // PIP
            if(mtx->owner->priority > current_task->priority)
                mtx->owner->priority = current_task->priority;

            // add to waiting queue
            if(mtx->waiting_count < MAX_WAITING_TASKS)
            {
                mtx->wait_queue[mtx->waiting_count] = current_task;
                mtx->waiting_count++;
                current_task->state = BLOCKED;
                critical_exit();
                scheduler_yield();
                return ;
            }
        }
        // if mutex is locked and waiting queue is full -> pooling
        critical_exit();
        scheduler_yield(); 
    }
}
void mutex_unlock(Mutex *mtx)
{
    critical_enter();
    if(mtx->owner == current_task)
    {
        current_task->priority = current_task->original_priority; // PIP restore
        if(mtx->waiting_count > 0)
        {
            uint32_t highest_prio_idx = 0;
            uint32_t highest_prio_val = mtx->wait_queue[0]->priority;
            for(uint32_t i = 1; i < mtx->waiting_count; i++)
            {
                if(mtx->wait_queue[i]->priority < highest_prio_val)
                {
                    highest_prio_idx = i;
                    highest_prio_val = mtx->wait_queue[i]->priority;
                }
            }
            TCB *next_owner = mtx->wait_queue[highest_prio_idx];
            for(uint32_t i = highest_prio_idx; i < mtx->waiting_count - 1; i++)
            {
                mtx->wait_queue[i] = mtx->wait_queue[i+1];
            }
            mtx->waiting_count--;
            mtx->owner = next_owner;
            next_owner->state = READY;
        }
        else
        {
            mtx->locked = 0;
            mtx->owner = NULL;
        }
    }
    critical_exit();
}