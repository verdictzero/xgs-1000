#include "core/memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// Global memory manager
MemoryManager g_memory_manager = {0};

// Align size to 16 bytes for SIMD and cache efficiency
static size_t align_size(size_t size) {
    return (size + 15) & ~15;
}

// Find first free block in bitfield
static int find_free_block(const unsigned char* free_list, size_t block_count) {
    for (size_t i = 0; i < block_count; i++) {
        size_t byte_index = i / 8;
        size_t bit_index = i % 8;
        
        if (!(free_list[byte_index] & (1 << bit_index))) {
            return (int)i;
        }
    }
    return -1; // No free blocks
}

// Set bit in bitfield
static void set_bit(unsigned char* free_list, size_t index, bool value) {
    size_t byte_index = index / 8;
    size_t bit_index = index % 8;
    
    if (value) {
        free_list[byte_index] |= (1 << bit_index);
    } else {
        free_list[byte_index] &= ~(1 << bit_index);
    }
}

// Check bit in bitfield
static bool get_bit(const unsigned char* free_list, size_t index) {
    size_t byte_index = index / 8;
    size_t bit_index = index % 8;
    
    return (free_list[byte_index] & (1 << bit_index)) != 0;
}

bool memory_pool_init(MemoryPool* pool, size_t block_size, size_t block_count, const char* name) {
    if (!pool || block_size == 0 || block_count == 0) {
        return false;
    }

    // Align block size to 16 bytes for SIMD
    block_size = align_size(block_size);

    pool->block_size = block_size;
    pool->block_count = block_count;
    pool->used_blocks = 0;
    pool->name = name ? name : "Unknown";

    // Allocate memory with alignment
    size_t total_size = block_size * block_count;
#ifdef _WIN32
    pool->memory = _aligned_malloc(total_size, 16);
#else
    pool->memory = aligned_alloc(16, total_size);
#endif

    if (!pool->memory) {
        fprintf(stderr, "Failed to allocate memory pool '%s': %zu bytes\n", pool->name, total_size);
        return false;
    }

    // Initialize free list (bitfield)
    size_t free_list_size = (block_count + 7) / 8;
    pool->free_list = calloc(free_list_size, 1);
    if (!pool->free_list) {
        fprintf(stderr, "Failed to allocate free list for pool '%s'\n", pool->name);
#ifdef _WIN32
        _aligned_free(pool->memory);
#else
        free(pool->memory);
#endif
        return false;
    }

    // Clear memory for debugging
    memset(pool->memory, 0, total_size);

    printf("Memory pool '%s' initialized: %zu blocks × %zu bytes = %zu KB\n",
           pool->name, block_count, block_size, total_size / 1024);

    return true;
}

void* memory_pool_alloc(MemoryPool* pool) {
    if (!pool || pool->used_blocks >= pool->block_count) {
        return NULL;
    }

    int free_index = find_free_block(pool->free_list, pool->block_count);
    if (free_index < 0) {
        fprintf(stderr, "Memory pool '%s' is full (%zu/%zu blocks used)\n", 
                pool->name, pool->used_blocks, pool->block_count);
        return NULL;
    }

    // Mark block as used
    set_bit(pool->free_list, free_index, true);
    pool->used_blocks++;

    // Calculate pointer
    void* ptr = (char*)pool->memory + (free_index * pool->block_size);
    
    // Clear block for consistency
    memset(ptr, 0, pool->block_size);

    return ptr;
}

void memory_pool_free(MemoryPool* pool, void* ptr) {
    if (!pool || !ptr) {
        return;
    }

    // Check if pointer belongs to this pool
    char* pool_start = (char*)pool->memory;
    char* pool_end = pool_start + (pool->block_count * pool->block_size);
    
    if (ptr < (void*)pool_start || ptr >= (void*)pool_end) {
        fprintf(stderr, "Attempting to free pointer not owned by pool '%s'\n", pool->name);
        return;
    }

    // Calculate block index
    ptrdiff_t offset = (char*)ptr - pool_start;
    if (offset % pool->block_size != 0) {
        fprintf(stderr, "Invalid pointer alignment in pool '%s'\n", pool->name);
        return;
    }

    size_t block_index = offset / pool->block_size;
    
    // Check if block is actually allocated
    if (!get_bit(pool->free_list, block_index)) {
        fprintf(stderr, "Double free detected in pool '%s' (block %zu)\n", pool->name, block_index);
        return;
    }

    // Mark block as free
    set_bit(pool->free_list, block_index, false);
    pool->used_blocks--;

    // Clear block to help catch use-after-free bugs
    memset(ptr, 0xDE, pool->block_size);
}

void memory_pool_destroy(MemoryPool* pool) {
    if (!pool) {
        return;
    }

    printf("Destroying memory pool '%s' (%zu/%zu blocks were used)\n", 
           pool->name, pool->used_blocks, pool->block_count);

    if (pool->memory) {
#ifdef _WIN32
        _aligned_free(pool->memory);
#else
        free(pool->memory);
#endif
        pool->memory = NULL;
    }

    if (pool->free_list) {
        free(pool->free_list);
        pool->free_list = NULL;
    }

    pool->block_size = 0;
    pool->block_count = 0;
    pool->used_blocks = 0;
}

size_t memory_pool_used_bytes(const MemoryPool* pool) {
    return pool ? pool->used_blocks * pool->block_size : 0;
}

size_t memory_pool_free_bytes(const MemoryPool* pool) {
    if (!pool) return 0;
    return (pool->block_count - pool->used_blocks) * pool->block_size;
}

float memory_pool_fragmentation(const MemoryPool* pool) {
    if (!pool || pool->block_count == 0) return 0.0f;
    return (float)pool->used_blocks / pool->block_count;
}

bool memory_pool_owns(const MemoryPool* pool, void* ptr) {
    if (!pool || !ptr) return false;
    
    char* pool_start = (char*)pool->memory;
    char* pool_end = pool_start + (pool->block_count * pool->block_size);
    
    return ptr >= (void*)pool_start && ptr < (void*)pool_end;
}

void memory_pool_reset(MemoryPool* pool) {
    if (!pool) return;
    
    size_t free_list_size = (pool->block_count + 7) / 8;
    memset(pool->free_list, 0, free_list_size);
    pool->used_blocks = 0;
    
    printf("Memory pool '%s' reset\n", pool->name);
}

bool memory_manager_init(MemoryManager* manager) {
    if (!manager) return false;
    
    memset(manager, 0, sizeof(MemoryManager));
    
    // Define pool sizes for different allocation patterns
    struct PoolConfig {
        size_t block_size;
        size_t block_count;
        const char* name;
    } configs[] = {
        {32,    1024,  "Small (32B)"},      // Small allocations
        {64,    512,   "Medium (64B)"},     // Medium allocations  
        {128,   256,   "Large (128B)"},     // Large allocations
        {256,   128,   "XLarge (256B)"},    // Extra large
        {512,   64,    "Huge (512B)"},      // Vertex data
        {1024,  32,    "Massive (1KB)"},    // Mesh data
        {4096,  16,    "Giant (4KB)"},      // Texture data
        {8192,  8,     "Colossal (8KB)"},   // Large buffers
    };
    
    const int num_configs = sizeof(configs) / sizeof(configs[0]);
    
    for (int i = 0; i < num_configs && i < MAX_MEMORY_POOLS; i++) {
        if (!memory_pool_init(&manager->pools[i], 
                             configs[i].block_size, 
                             configs[i].block_count,
                             configs[i].name)) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                memory_pool_destroy(&manager->pools[j]);
            }
            return false;
        }
        manager->pool_count++;
    }
    
    printf("Memory manager initialized with %d pools\n", manager->pool_count);
    return true;
}

void memory_manager_destroy(MemoryManager* manager) {
    if (!manager) return;
    
    for (int i = 0; i < manager->pool_count; i++) {
        memory_pool_destroy(&manager->pools[i]);
    }
    
    memset(manager, 0, sizeof(MemoryManager));
    printf("Memory manager destroyed\n");
}

void* memory_manager_alloc(MemoryManager* manager, size_t size) {
    if (!manager || size == 0) return NULL;
    
    // Find the smallest pool that can fit this allocation
    for (int i = 0; i < manager->pool_count; i++) {
        MemoryPool* pool = &manager->pools[i];
        if (pool->block_size >= size) {
            void* ptr = memory_pool_alloc(pool);
            if (ptr) {
                manager->stats.total_allocated += pool->block_size;
                if (manager->stats.total_allocated > manager->stats.peak_usage) {
                    manager->stats.peak_usage = manager->stats.total_allocated;
                }
                return ptr;
            }
        }
    }
    
    fprintf(stderr, "Failed to allocate %zu bytes - no suitable pool found\n", size);
    return NULL;
}

void memory_manager_free(MemoryManager* manager, void* ptr) {
    if (!manager || !ptr) return;
    
    // Find which pool owns this pointer
    for (int i = 0; i < manager->pool_count; i++) {
        MemoryPool* pool = &manager->pools[i];
        if (memory_pool_owns(pool, ptr)) {
            memory_pool_free(pool, ptr);
            manager->stats.total_allocated -= pool->block_size;
            return;
        }
    }
    
    fprintf(stderr, "Attempting to free pointer not owned by memory manager\n");
}

MemoryStats memory_manager_get_stats(const MemoryManager* manager) {
    MemoryStats stats = {0};
    if (!manager) return stats;
    
    stats = manager->stats;
    
    // Calculate current usage and fragmentation
    size_t total_size = 0;
    size_t used_size = 0;
    
    for (int i = 0; i < manager->pool_count; i++) {
        const MemoryPool* pool = &manager->pools[i];
        total_size += pool->block_count * pool->block_size;
        used_size += memory_pool_used_bytes(pool);
    }
    
    stats.total_used = used_size;
    stats.fragmentation = total_size > 0 ? (float)used_size / total_size : 0.0f;
    
    return stats;
}

void memory_manager_print_stats(const MemoryManager* manager) {
    if (!manager) return;
    
    printf("\n=== Memory Manager Statistics ===\n");
    
    for (int i = 0; i < manager->pool_count; i++) {
        const MemoryPool* pool = &manager->pools[i];
        size_t used = memory_pool_used_bytes(pool);
        size_t total = pool->block_count * pool->block_size;
        float usage = total > 0 ? (float)used / total * 100.0f : 0.0f;
        
        printf("Pool %-15s: %3zu/%3zu blocks (%5.1f%%) = %6zu/%6zu bytes\n",
               pool->name,
               pool->used_blocks, pool->block_count,
               usage,
               used, total);
    }
    
    MemoryStats stats = memory_manager_get_stats(manager);
    printf("\nTotal usage: %zu KB (peak: %zu KB)\n", 
           stats.total_used / 1024, stats.peak_usage / 1024);
    printf("Fragmentation: %.1f%%\n", stats.fragmentation * 100.0f);
    printf("================================\n\n");
}