#include "kernel/memory_pool.h"
#include "kernel/scheduler.h"
#include <stdint.h>

void pool_init(MemoryPool *pool, void *buffer, uint32_t block_size, uint32_t total_blocks)
{
    pool->pool_buffer = (uint8_t*)buffer;
    pool->block_size = block_size;
    pool->total_blocks = total_blocks;
    pool->free_list_head = buffer;
    pool->free_blocks = total_blocks;

    uint8_t* current_block = (uint8_t*)buffer;
    for(uint32_t i = 0; i < total_blocks - 1; i++)
    {
        uint8_t* next_block = current_block + block_size;
        *((void **)current_block) = next_block; // equivalent to *((uint32_t*)current_block) = (uint32_t)next_block;
        current_block = next_block;
    }
    *((void **)current_block) = NULL;
}

void *pool_alloc(MemoryPool *pool)
{
    critical_enter();
    if(pool->free_list_head == NULL)
    {
        critical_exit();
        return NULL;
    }
    void *allocated_block = pool->free_list_head;
    pool->free_list_head = *((void **)allocated_block);
    pool->free_blocks--;
    critical_exit();
    return allocated_block;
}

void pool_free(MemoryPool *pool, void *block)
{
    critical_enter();
    if(block == NULL)
    {
        critical_exit();
        return;
    }    
    *((void **)block) = pool->free_list_head;
    pool->free_list_head = block;
    pool->free_blocks++;
    critical_exit();
}

uint32_t pool_get_free_blocks(MemoryPool *pool)
{
    return pool->free_blocks;
}