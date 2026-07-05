#ifndef KERNEL_MEMORY_POOL_H
#define KERNEL_MEMORY_POOL_H

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint8_t *pool_buffer;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t free_blocks;
    void *free_list_head;   // point to the first free block
} MemoryPool;

void pool_init(MemoryPool *pool, void *buffer, uint32_t block_size, uint32_t total_blocks);
void* pool_alloc(MemoryPool *pool);
void pool_free(MemoryPool *pool, void *block);
uint32_t pool_get_free_blocks(MemoryPool *pool);

#endif