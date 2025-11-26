# Tests for the fantasy console runtime

# Test sources
set(TEST_SOURCES
    test_runner.c
    test_memory.c
    test_math.c
    # test_collision.c
    # test_physics.c
    # test_scene_loader.c
)

# Create test executable
add_executable(runtime_tests ${TEST_SOURCES})

# Link with main runtime sources (excluding main.c)
file(GLOB_RECURSE RUNTIME_LIB_SOURCES "../src/*.c" "../src/*.cpp")
list(FILTER RUNTIME_LIB_SOURCES EXCLUDE REGEX ".*main\\.c$")

target_sources(runtime_tests PRIVATE ${RUNTIME_LIB_SOURCES})

# Include directories
target_include_directories(runtime_tests PRIVATE ../include)

# Link libraries
target_link_libraries(runtime_tests 
    ${GRAPHICS_LIBS}
    ${AUDIO_LIBS}
    m
)

# Platform-specific definitions
if(RASPBERRY_PI)
    target_compile_definitions(runtime_tests PRIVATE RASPBERRY_PI=1)
    target_compile_definitions(runtime_tests PRIVATE USE_OPENGL_ES=1)
else()
    target_compile_definitions(runtime_tests PRIVATE DESKTOP=1)
endif()

# Add test
add_test(NAME RuntimeTests COMMAND runtime_tests)

# Test properties
set_tests_properties(RuntimeTests PROPERTIES TIMEOUT 30)