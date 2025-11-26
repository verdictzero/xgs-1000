#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MemoryPool {
    void* memory;
    size_t block_size;
    size_t block_count;
    size_t used_blocks;
    unsigned char* free_list;
    const char* name; // For debugging
} MemoryPool;

typedef struct MemoryStats {
    size_t total_allocated;
    size_t total_used;
    size_t peak_usage;
    float fragmentation;
} MemoryStats;

// Initialize pool with fixed block size
bool memory_pool_init(MemoryPool* pool, size_t block_size, size_t block_count, const char* name);

// Allocate from pool
void* memory_pool_alloc(MemoryPool* pool);

// Free back to pool
void memory_pool_free(MemoryPool* pool, void* ptr);

// Destroy pool
void memory_pool_destroy(MemoryPool* pool);

// Get stats
size_t memory_pool_used_bytes(const MemoryPool* pool);
size_t memory_pool_free_bytes(const MemoryPool* pool);
float memory_pool_fragmentation(const MemoryPool* pool);

// Check if pointer belongs to this pool
bool memory_pool_owns(const MemoryPool* pool, void* ptr);

// Reset pool (mark all blocks as free)
void memory_pool_reset(MemoryPool* pool);

// Memory manager - manages multiple pools
#define MAX_MEMORY_POOLS 16

typedef struct MemoryManager {
    MemoryPool pools[MAX_MEMORY_POOLS];
    int pool_count;
    MemoryStats stats;
} MemoryManager;

// Initialize memory manager with predefined pools
bool memory_manager_init(MemoryManager* manager);

// Destroy memory manager
void memory_manager_destroy(MemoryManager* manager);

// Allocate from appropriate pool
void* memory_manager_alloc(MemoryManager* manager, size_t size);

// Free memory (finds the right pool automatically)
void memory_manager_free(MemoryManager* manager, void* ptr);

// Get memory statistics
MemoryStats memory_manager_get_stats(const MemoryManager* manager);

// Print debug information
void memory_manager_print_stats(const MemoryManager* manager);

// Global memory manager instance
extern MemoryManager g_memory_manager;

// Convenience macros
#define FC_MALLOC(size) memory_manager_alloc(&g_memory_manager, size)
#define FC_FREE(ptr) memory_manager_free(&g_memory_manager, ptr)

#ifdef __cplusplus
}
#endif

#endif // MEMORY_H