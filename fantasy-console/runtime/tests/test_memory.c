#include "core/memory.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_basic_allocation() {
    MemoryPool pool;
    assert(memory_pool_init(&pool, 64, 100, "test_pool"));

    void* ptr1 = memory_pool_alloc(&pool);
    assert(ptr1 != NULL);

    void* ptr2 = memory_pool_alloc(&pool);
    assert(ptr2 != NULL);
    assert(ptr1 != ptr2);

    // Check alignment (should be 16-byte aligned)
    assert(((uintptr_t)ptr1 % 16) == 0);
    assert(((uintptr_t)ptr2 % 16) == 0);

    memory_pool_free(&pool, ptr1);
    memory_pool_free(&pool, ptr2);

    memory_pool_destroy(&pool);
    printf("✓ Basic allocation test passed\n");
}

void test_pool_exhaustion() {
    MemoryPool pool;
    assert(memory_pool_init(&pool, 32, 5, "small_pool"));

    void* ptrs[10];
    int allocated = 0;

    // Try to allocate more than the pool can hold
    for (int i = 0; i < 10; i++) {
        ptrs[i] = memory_pool_alloc(&pool);
        if (ptrs[i]) {
            allocated++;
        }
    }

    assert(allocated == 5); // Should only allocate 5
    assert(memory_pool_alloc(&pool) == NULL); // Should fail

    // Free all
    for (int i = 0; i < allocated; i++) {
        memory_pool_free(&pool, ptrs[i]);
    }

    memory_pool_destroy(&pool);
    printf("✓ Pool exhaustion test passed\n");
}

void test_memory_reuse() {
    MemoryPool pool;
    assert(memory_pool_init(&pool, 64, 10, "reuse_pool"));

    void* ptr1 = memory_pool_alloc(&pool);
    assert(ptr1 != NULL);

    memory_pool_free(&pool, ptr1);

    void* ptr2 = memory_pool_alloc(&pool);
    assert(ptr2 != NULL);

    // Should reuse the same memory
    assert(ptr1 == ptr2);

    memory_pool_free(&pool, ptr2);
    memory_pool_destroy(&pool);
    printf("✓ Memory reuse test passed\n");
}

void test_memory_manager() {
    MemoryManager manager;
    assert(memory_manager_init(&manager));

    // Test different size allocations
    void* small = memory_manager_alloc(&manager, 16);
    void* medium = memory_manager_alloc(&manager, 100);
    void* large = memory_manager_alloc(&manager, 1000);

    assert(small != NULL);
    assert(medium != NULL);
    assert(large != NULL);

    // Write some data to ensure memory is valid
    strcpy((char*)small, "small");
    strcpy((char*)medium, "medium allocation");
    strcpy((char*)large, "large allocation test data");

    // Verify data
    assert(strcmp((char*)small, "small") == 0);
    assert(strcmp((char*)medium, "medium allocation") == 0);
    assert(strcmp((char*)large, "large allocation test data") == 0);

    memory_manager_free(&manager, small);
    memory_manager_free(&manager, medium);
    memory_manager_free(&manager, large);

    memory_manager_destroy(&manager);
    printf("✓ Memory manager test passed\n");
}

void test_statistics() {
    MemoryManager manager;
    assert(memory_manager_init(&manager));

    void* ptr1 = memory_manager_alloc(&manager, 32);
    void* ptr2 = memory_manager_alloc(&manager, 64);
    void* ptr3 = memory_manager_alloc(&manager, 128);

    MemoryStats stats = memory_manager_get_stats(&manager);
    assert(stats.total_used > 0);
    assert(stats.fragmentation > 0.0f);

    memory_manager_print_stats(&manager);

    memory_manager_free(&manager, ptr1);
    memory_manager_free(&manager, ptr2);
    memory_manager_free(&manager, ptr3);

    memory_manager_destroy(&manager);
    printf("✓ Statistics test passed\n");
}

void test_boundary_conditions() {
    MemoryPool pool;
    
    // Test invalid parameters
    assert(!memory_pool_init(&pool, 0, 10, "invalid"));
    assert(!memory_pool_init(&pool, 10, 0, "invalid"));
    assert(!memory_pool_init(NULL, 10, 10, "invalid"));

    // Test valid pool
    assert(memory_pool_init(&pool, 64, 2, "boundary"));

    void* ptr1 = memory_pool_alloc(&pool);
    void* ptr2 = memory_pool_alloc(&pool);
    void* ptr3 = memory_pool_alloc(&pool); // Should fail

    assert(ptr1 != NULL);
    assert(ptr2 != NULL);
    assert(ptr3 == NULL);

    memory_pool_free(&pool, ptr1);
    memory_pool_free(&pool, ptr2);
    
    // Test double free (should not crash)
    memory_pool_free(&pool, ptr1);

    // Test freeing NULL
    memory_pool_free(&pool, NULL);

    memory_pool_destroy(&pool);
    printf("✓ Boundary conditions test passed\n");
}

void run_memory_tests() {
    printf("Running memory system tests...\n");
    
    test_basic_allocation();
    test_pool_exhaustion();
    test_memory_reuse();
    test_memory_manager();
    test_statistics();
    test_boundary_conditions();
    
    printf("All memory tests passed!\n\n");
}