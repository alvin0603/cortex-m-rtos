#include "kernel/semaphore.h"
#include <stdint.h>

void sem_init(Semaphore *sem, uint32_t init_count)
{
    sem->count = init_count;
    sem->waiting_count = 0;
}

void sem_wait(Semaphore *sem)
{
    critical_enter();
    if(sem->count > 0)
    {
        sem->count--;
        critical_exit();
        return;
    }
    sem->wait_queue[sem->waiting_count++] = current_task;
    current_task->state = BLOCKED;
    critical_exit();
    scheduler_yield();
}

void sem_post(Semaphore *sem)
{
    critical_enter();
    if(sem->waiting_count > 0)
    {
        sem->wait_queue[0]->state = READY;
        for(uint32_t i = 0; i < sem->waiting_count - 1; i++)
            sem->wait_queue[i] = sem->wait_queue[i+1];
        sem->waiting_count--;
    }
    else
        sem->count++;
    critical_exit();
}
