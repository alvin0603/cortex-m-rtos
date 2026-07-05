#ifndef INCLUDE_KERNEL_MESSAGE_QUEUE_H
#define INCLUDE_KERNEL_MESSAGE_QUEUE_H

#include "kernel/task.h"
#include "kernel/semaphore.h"
#include <stdint.h>


typedef struct
{
    uint32_t *buffer;
    uint32_t capacity;
    uint32_t head;
    uint32_t tail;
    Semaphore empty_slots; 
    Semaphore filled_slots;
} MessageQueue;

void msg_queue_init(MessageQueue *mq, uint32_t *buffer, uint32_t capacity);
void msg_queue_send(MessageQueue *mq, uint32_t msg);
uint8_t msg_queue_send_isr(MessageQueue *mq, uint32_t msg);
void msg_queue_receive(MessageQueue *mq, uint32_t *msg);

#endif