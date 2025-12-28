# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### Windows

```sh
# Configure
cmake --preset windows-debug
cmake --preset windows-developer-release
cmake --preset windows-release

# Build
cmake --build --preset windows-debug
cmake --build --preset windows-developer-release
cmake --build --preset windows-release
```

### Linux

```sh
# Configure
cmake --preset linux-debug
cmake --preset linux-developer-release
cmake --preset linux-release

# Build
cmake --build --preset linux-debug
cmake --build --preset linux-developer-release
cmake --build --preset linux-release
```

### Web (Emscripten)

```sh
# Windows
cmake --preset web-debug-on-windows
cmake --build --preset web-debug-on-windows

# Linux
cmake --preset web-debug
cmake --build --preset web-debug
```

### Automated Build Script

For scanning and building all configurations:

```sh
python3 scripts/scan_build_project.py
python3 scripts/scan_build_project.py --help  # See filtering options
```

## Build Configurations

- **Debug**: No optimizations, breakpoint support, console enabled, all developer features. For active development.
- **Developer-Release**: Optimized code with console and verbose logging. For performance testing with diagnostics.
- **Release**: Fully optimized, no console, minimal logging. For end-user distribution.

The `IS_DEVELOPER_VERSION` CMake variable controls whether `DEVELOPER_VERSION` is defined, which affects logging, assertions, and OpenGL source location tracking.

## Architecture Overview

### Core Engine Pattern: Singleton with Pimpl

The [Engine](source/Engine/Engine.h) class is the central singleton managing all subsystems. It uses the Pimpl idiom to hide implementation details. Access subsystems through static methods:

```cpp
Engine::GetLogger()
Engine::GetWindow()
Engine::GetInput()
Engine::GetGameStateManager()
Engine::GetTextureManager()
Engine::GetWindowEnvironment()  // Timing and frame data
```

### Application Lifecycle

1. `Engine::Instance().Start("Title")` - Initialize all subsystems
2. `Engine::GetGameStateManager().PushState<YourState>()` - Set initial state
3. Main loop: `engine.Update()` until `engine.HasGameEnded()`
4. `engine.Stop()` - Clean shutdown

For web builds, use `emscripten_set_main_loop()` instead of a while loop (see [main.cpp](source/main.cpp:27-81)).

### Game State System

States inherit from [CS230::GameState](source/Engine/GameState.h) and implement:

- `Load()` - Initialize state resources
- `Update(double dt)` - Frame logic
- `Draw()` - Render calls
- `DrawImGui()` - Debug UI
- `Unload()` - Cleanup
- `GetName()` - State identifier

States use a component system via `ComponentManager`:

- `AddGSComponent(Component*)` - Attach state-level components
- `GetGSComponent<T>()` - Retrieve components
- `UpdateGSComponents(dt)` - Update all components

### GameObject Architecture

[GameObject](source/Engine/GameObject.h) is the base class for all game entities. Key features:

- **Transform**: Position, rotation, scale with `TransformationMatrix`
- **Component System**: Attach components via `ComponentManager`
- **State Machine**: Internal state pattern with `State` interface
- **Collision**: `IsCollidingWith()`, `CanCollideWith()`, `ResolveCollision()`
- **Draw Priority**: Control render order with `DrawPriority()` (higher = later/upper layer, typically 30-70)
- **Update Priority**: Control update order with `UpdatePriority()`

GameObjects are managed by `GameObjectManager` which handles updates, drawing, and cleanup.

### 2D Rendering System

The rendering system has three implementations of [IRenderer2D](source/CS200/IRenderer2D.h):

1. **ImmediateRenderer2D** - Simple immediate mode rendering
2. **BatchRenderer2D** - Batches draw calls for performance (default: 10,000 quads max)
3. **InstancedRenderer2D** - Instanced rendering for many identical objects

All renderers follow the pattern:

```cpp
renderer.BeginScene(camera_matrix);
renderer.DrawQuad(transform, texture, uv_bl, uv_tr, tint, depth);
renderer.DrawCircle(transform, fill, line_color, line_width, depth);
renderer.DrawRectangle(transform, fill, line_color, line_width, depth);
renderer.DrawLine(start, end, color, width, depth);
renderer.EndScene();  // Triggers actual rendering
```

Coordinate system transformations handled by camera matrices passed to `BeginScene()`.

### BatchRenderer2D Implementation Details

- Accumulates vertices until buffer full or `EndScene()` called
- Supports texture batching with multiple texture slots
- Uses SDF (Signed Distance Field) rendering for circles/rectangles with anti-aliased outlines
- Separates textured quad rendering from SDF shape rendering with different shaders
- Uniform buffer for camera matrix shared across draw calls

### OpenGL Wrapper

The [GL](source/OpenGL/GL.h) namespace wraps raw OpenGL calls with:

- **Source Location Tracking**: In developer builds, all GL calls record file/line for debugging
- **Error Checking**: Automatic error reporting in developer builds
- **Type Safety**: Wrapped handles (TextureHandle, ShaderHandle, etc.) via [Handle.h](source/OpenGL/Handle.h)

Core OpenGL wrappers:

- [Buffer.h](source/OpenGL/Buffer.h) - VBO/IBO management
- [VertexArray.h](source/OpenGL/VertexArray.h) - VAO management
- [Shader.h](source/OpenGL/Shader.h) - Shader compilation and linking
- [Texture.h](source/OpenGL/Texture.h) - Texture loading and binding
- [Framebuffer.h](source/OpenGL/Framebuffer.h) - FBO management

### Component System

Two separate component systems exist:

1. **GameObject Components** - Attached to individual GameObjects (e.g., Sprite, Collision, Animation)
2. **GameState Components** - Attached to GameStates (e.g., GameObjectManager, Camera)

Both use [ComponentManager](source/Engine/ComponentManager.h) which stores components by type and calls `Update(dt)`.

### Asset Management

- **Textures**: Loaded via `TextureManager` with automatic caching
- **Fonts**: Managed by `Font` and `TextManager` classes
- **Shaders**: Located in [Assets/shaders/](Assets/shaders/)
- **Images**: Located in [Assets/images/](Assets/images/)
- **Sprites**: Animation data in [Assets/sprites/](Assets/sprites/)

Assets are embedded in web builds via `--embed-file` in CMake.

## Directory Structure

- `source/Engine/` - Core engine systems (Window, Input, GameState, GameObject, etc.)
- `source/OpenGL/` - OpenGL wrapper classes and utilities
- `source/CS200/` - 2D rendering implementations and utilities
- `source/Demo/` - Various rendering demos and examples
- `source/Game/` - Game-specific states and objects
- `Assets/` - Textures, shaders, fonts, sprites, animations
- `cmake/` - CMake configuration files
- `cmake/dependencies/` - Third-party dependency management

## Dependencies

Managed via CMake FetchContent in [cmake/Dependencies.cmake](cmake/Dependencies.cmake):

- **OpenGL** - Graphics API
- **GLEW** - OpenGL extension wrangler
- **SDL2** - Window and input management
- **Dear ImGui** - Debug UI
- **GSL** - Guidelines Support Library (for `gsl::owner`, `gsl::czstring`, etc.)
- **STB** - Image loading (stb_image)

## Common Development Patterns

### Creating a New Game State

1. Create header/cpp files inheriting from `CS230::GameState`
2. Implement all pure virtual methods
3. Add components in `Load()` using `AddGSComponent()`
4. Push state: `Engine::GetGameStateManager().PushState<YourState>()`

### Creating a New GameObject Type

1. Inherit from `CS230::GameObject`
2. Override `Type()` and `TypeName()` (define type in [GameObjectTypes.h](Game/GameObjectTypes.h))
3. Override `DrawPriority()` if rendering order matters
4. Implement collision methods if needed
5. Add to `GameObjectManager` in your game state

### Coordinate Systems

- Engine uses custom [Vec2](source/Engine/Vec2.h) and [Matrix](source/Engine/Matrix.h) classes
- [TransformationMatrix](source/Engine/Matrix.h) handles 2D transforms (translate, rotate, scale)
- [NDC.h](source/CS200/NDC.h) contains Normalized Device Coordinate utilities
- Camera matrices transform world space to NDC space

### DEVELOPER_VERSION Preprocessor Flag

When defined (Debug and Developer-Release builds):

- OpenGL calls track source location for debugging
- Verbose logging enabled
- Console window visible
- ImGui debug UI available
- Assertions active

Check with `#if defined(DEVELOPER_VERSION)` when adding developer-only features.

## Code Style

The project follows C++20 standards with:

- Classes in PascalCase
- Functions/methods in camelCase
- Member variables in snake_case
- Header files use `.h` extension (consistent across entire codebase)
- Namespace organization: `CS230` for engine, `CS200` for rendering, `Math` for math utilities
- Doxygen-style documentation comments for public APIs
- `.clang-format` file defines formatting rules
