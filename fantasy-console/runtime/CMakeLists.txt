cmake_minimum_required(VERSION 3.18)
project(FantasyConsoleRuntime C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Platform detection
option(RASPBERRY_PI "Build for Raspberry Pi" OFF)

if(RASPBERRY_PI)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a -mfpu=neon-fp-armv8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a -mfpu=neon-fp-armv8")
    
    # OpenGL ES for Pi
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLES2 REQUIRED glesv2)
    pkg_check_modules(EGL REQUIRED egl)
    
    # Broadcom VideoCore
    find_library(BCM_HOST_LIBS bcm_host /opt/vc/lib)
    find_path(BCM_HOST_INCLUDE_DIR bcm_host.h /opt/vc/include)
    
    set(GRAPHICS_LIBS ${GLES2_LIBRARIES} ${EGL_LIBRARIES} ${BCM_HOST_LIBS})
    set(GRAPHICS_INCLUDE_DIRS ${GLES2_INCLUDE_DIRS} ${EGL_INCLUDE_DIRS} ${BCM_HOST_INCLUDE_DIR})
else()
    # Desktop OpenGL
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    
    set(GRAPHICS_LIBS ${OPENGL_LIBRARIES} glfw)
    set(GRAPHICS_INCLUDE_DIRS ${OPENGL_INCLUDE_DIRS})
endif()

# Audio
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(OPENAL openal)
endif()

if(OPENAL_FOUND)
    set(AUDIO_LIBS ${OPENAL_LIBRARIES})
    set(AUDIO_INCLUDE_DIRS ${OPENAL_INCLUDE_DIRS})
else()
    # Fallback to system OpenAL
    find_library(OPENAL_LIBRARY openal)
    find_path(OPENAL_INCLUDE_DIR AL/al.h)
    set(AUDIO_LIBS ${OPENAL_LIBRARY})
    set(AUDIO_INCLUDE_DIRS ${OPENAL_INCLUDE_DIR})
endif()

# Math library (GLM)
find_package(glm QUIET)
if(NOT glm_FOUND)
    # GLM is header-only, so we can include it directly
    find_path(GLM_INCLUDE_DIR glm/glm.hpp)
    if(GLM_INCLUDE_DIR)
        set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})
    endif()
endif()

# Include directories
include_directories(include)
include_directories(${GRAPHICS_INCLUDE_DIRS})
include_directories(${AUDIO_INCLUDE_DIRS})
if(GLM_INCLUDE_DIRS)
    include_directories(${GLM_INCLUDE_DIRS})
endif()

# Source files
file(GLOB_RECURSE RUNTIME_SOURCES "src/*.c" "src/*.cpp")

# Create executable
add_executable(fantasy_console ${RUNTIME_SOURCES})

# Link libraries
target_link_libraries(fantasy_console 
    ${GRAPHICS_LIBS}
    ${AUDIO_LIBS}
    m  # Math library
)

# Compiler flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(fantasy_console PRIVATE DEBUG=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra")
else()
    target_compile_definitions(fantasy_console PRIVATE NDEBUG=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
endif()

# Platform-specific definitions
if(RASPBERRY_PI)
    target_compile_definitions(fantasy_console PRIVATE RASPBERRY_PI=1)
    target_compile_definitions(fantasy_console PRIVATE USE_OPENGL_ES=1)
else()
    target_compile_definitions(fantasy_console PRIVATE DESKTOP=1)
endif()

# Enable NEON if available
include(CheckCCompilerFlag)
check_c_compiler_flag(-mfpu=neon HAS_NEON)
if(HAS_NEON)
    target_compile_definitions(fantasy_console PRIVATE HAS_NEON=1)
endif()

# Add tests
enable_testing()
add_subdirectory(tests)

# Install target
install(TARGETS fantasy_console DESTINATION bin)

# Copy shaders
install(DIRECTORY shaders/ DESTINATION share/fantasy-console/shaders)

# Status messages
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target platform: ${CMAKE_SYSTEM_NAME}")
if(RASPBERRY_PI)
    message(STATUS "Building for Raspberry Pi")
    message(STATUS "Graphics: OpenGL ES 2.0")
else()
    message(STATUS "Building for Desktop")
    message(STATUS "Graphics: OpenGL")
endif()
message(STATUS "Audio: ${AUDIO_LIBS}")
message(STATUS "Graphics: ${GRAPHICS_LIBS}")