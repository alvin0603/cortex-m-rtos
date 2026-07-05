#include "kernel/message_queue.h"
#include "kernel/semaphore.h"
#include "kernel/scheduler.h"
#include <stdint.h>

void msg_queue_init(MessageQueue *mq, uint32_t *buffer, uint32_t capacity)
{
    sem_init(&mq->empty_slots, capacity);
    sem_init(&mq->filled_slots, 0);
    mq->buffer = buffer;
    mq->capacity = capacity;
    mq->head = 0;
    mq->tail = 0;
}
void msg_queue_send(MessageQueue *mq, uint32_t msg)
{
    sem_wait(&mq->empty_slots);
    critical_enter();
    mq->buffer[mq->head] = msg;
    mq->head = (mq->head + 1) % mq->capacity;
    critical_exit();
    sem_post(&mq->filled_slots);
}
uint8_t msg_queue_send_isr(MessageQueue *mq, uint32_t msg)
{
    if(sem_try_wait(&mq->empty_slots) == 0)
        return 0; // discard message to prevent ISR from deadlock
    critical_enter();
    mq->buffer[mq->head] = msg;
    mq->head = (mq->head + 1) % mq->capacity;
    critical_exit();
    sem_post(&mq->filled_slots);
    return 1;
}
void msg_queue_receive(MessageQueue *mq, uint32_t *msg)
{
    sem_wait(&mq->filled_slots);
    critical_enter();
    *msg = mq->buffer[mq->tail];
    mq->tail = (mq->tail + 1) % mq->capacity;
    critical_exit();
    sem_post(&mq->empty_slots);
}