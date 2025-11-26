# Fantasy Console Runtime

C/C++ runtime engine optimized for Raspberry Pi Zero 2 W.

## Building

### Desktop (for development and testing)

```bash
mkdir build
cd build
cmake ..
make -j4
```

### Raspberry Pi Zero 2 W (cross-compile)

```bash
mkdir build-arm
cd build-arm
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-toolchain.cmake -DRASPBERRY_PI=ON
make -j4
```

### Prerequisites

#### Desktop
- OpenGL 3.3+
- GLFW3
- OpenAL
- GLM (optional, math library)

#### Raspberry Pi
- OpenGL ES 2.0
- EGL
- Broadcom VideoCore libraries
- Cross-compilation toolchain

## Features

- Fixed-memory allocation (no malloc/free during runtime)
- ARM NEON optimized math
- LOD system for performance
- Simple physics
- 4-channel audio mixing
- Binary scene format (.fcs)

## Performance Targets

- 25fps minimum on Pi Zero 2 W
- 50,000 triangles maximum
- Sub-16ms frame times
- Fixed 16MB memory budget

## Architecture

```
├── core/           # Memory, math, utilities
├── renderer/       # OpenGL ES rendering
├── physics/        # Collision and dynamics
├── scene/          # Entity system, transforms
├── audio/          # Audio mixing and playback
└── platform/       # Platform abstraction
```