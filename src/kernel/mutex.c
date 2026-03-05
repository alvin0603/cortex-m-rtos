#include "kernel/mutex.h"
#include "kernel/scheduler.h"
#include <stddef.h>

void mutex_init(Mutex *mtx)
{
    mtx->locked = 0;
    mtx->owner = NULL;
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
        critical_exit();
        scheduler_yield();
    }
}
void mutex_unlock(Mutex *mtx)
{
    critical_enter();
    if(mtx->owner == current_task)
    {
        mtx->locked = 0;
        mtx->owner = NULL;
    }
    critical_exit();
}