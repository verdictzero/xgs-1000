#include <stdio.h>
#include <stdlib.h>

// Test function declarations
extern void run_memory_tests();
// extern void run_math_tests();
// extern void run_collision_tests();
// extern void run_physics_tests();
// extern void run_scene_loader_tests();

int main() {
    printf("=== Fantasy Console Runtime Tests ===\n\n");
    
    run_memory_tests();
    
    // TODO: Add more tests as systems are implemented
    // run_math_tests();
    // run_collision_tests();
    // run_physics_tests();
    // run_scene_loader_tests();
    
    printf("All tests completed successfully!\n");
    return 0;
}