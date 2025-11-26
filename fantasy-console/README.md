# Fantasy Console

A retro-style 3D fantasy console that runs on Raspberry Pi Zero 2 W with a desktop editor.

## Architecture

- **Runtime**: C/C++ engine optimized for Pi Zero 2 W
- **Editor**: Python/PySide6 desktop application for scene creation
- **Target Performance**: 25fps, 50k triangles, 512-color palette

## Project Structure

```
fantasy-console/
├── runtime/                    # C/C++ runtime for Pi Zero 2 W
│   ├── src/
│   │   ├── core/              # Memory management, math
│   │   ├── renderer/          # OpenGL ES rendering
│   │   ├── physics/           # Collision detection, rigid bodies
│   │   ├── scene/             # Entity system, transforms
│   │   ├── audio/             # 4-channel audio mixing
│   │   └── platform/          # Platform-specific code
│   ├── include/               # Header files
│   ├── shaders/               # GLSL shaders
│   └── tests/                 # Unit tests
├── editor/                    # Python desktop editor
│   ├── src/editor/
│   │   ├── ui/                # Qt interface
│   │   ├── tools/             # Asset importers
│   │   ├── compiler/          # Scene compiler
│   │   └── widgets/           # Custom widgets
│   └── resources/             # UI assets
├── specs/                     # File format specifications
├── examples/                  # Example projects
├── tools/                     # Build scripts
└── docs/                      # Documentation
```

## Getting Started

1. Build the runtime for your platform
2. Install the desktop editor
3. Open an example project
4. Export to Pi Zero 2 W

## File Formats

- `.fca`: Human-readable JSON scene format (editor)
- `.fcs`: Binary scene format (runtime)
- 512-color indexed textures
- Custom mesh format with LOD support

## Performance Targets

- 25fps minimum on Pi Zero 2 W
- 50,000 triangles per frame
- 512-color palette limitation
- 4-channel audio mixing
- Fixed memory pools (no dynamic allocation)