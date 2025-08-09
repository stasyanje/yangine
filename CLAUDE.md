# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**App** contains an **Engine** which is a DirectX 12-based graphics engine written in C++20. It's a Windows-specific project that builds a static library (`engine.lib`) and includes a sample application demonstrating triangle rendering.

## Build System

### Commands

- **Build app**: `make build` - Compiles all source files and creates `engine.lib` in the `build/` directory
- **CMake generate**: `make generate` - Creates Visual Studio solution using CMake in `build_cmake/` directory
- **Build application**: `make run` - Builds the library then compiles the MSBuild solution (requires SOLUTION FILE to be specified)
- **Clean**: `make clean` - Removes the entire `build/` directory
- **Format code**: `make format` - Runs `clang-format` on all `.cpp` and `.h` files
- **Analyze code**: `make analyze` - Runs `clang-tidy` on all `.cpp` and `.h` files

### Build Details

The project uses Microsoft Visual C++ compiler (`cl.exe`) with these key settings:
    - C++20 standard (`/std:c++20`)
    - Debug runtime (`/MDd`)
    - Warning level 4 (`/W4`)
    - Exception handling enabled (`/EHsc`)

### Dependencies

The build system includes extensive Windows SDK and Microsoft GDK paths for:
    - DirectX 12 SDK
    - Windows Runtime (WinRT)
    - Xbox Game Development Kit
    - WinPixEvent Runtime for profiling

## Architecture

### Core Components

1. **App** (`modules/app/main.cpp`): Main application class that manages initialization, message loop, and shutdown
2. **Engine** (`modules/engine/.cpp`): Main engine class that manages message loop around engine components
3. **Renderer** (`modules/Renderer.h/cpp`): DirectX 12 renderer implementing `DX::IDeviceNotify` interface, handles triangle rendering
4. **DeviceResources** (`modules/DeviceResources.h/cpp`): Wrapper for Direct3D 12 device, command queue, swap chain, and resource management
5. **WindowManager** (`modules/WindowManager.h/cpp`): Handles Win32 window creation and management

### Key Patterns

- Uses RAII and smart pointers (`std::unique_ptr`, `Microsoft::WRL::ComPtr`)
- Implements device lost/restored pattern through `IDeviceNotify` interface
- Follows DirectX 12 best practices with command allocators per back buffer
- Uses precompiled headers (`pch.h`) for performance

### Directory Structure

- `modules/engine/`: Core engine source files
- `include/IEngine.h`: Public header
- `vendor/engine/`: Third-party libraries (DirectX headers, WinPixEvent)
- `cmake/Debug`: Generated build artifacts
- `scripts/`: Build scripts (primarily `build.bat`)

### Graphics Pipeline

The renderer implements a basic triangle rendering pipeline with:
    - Root signature and pipeline state objects
    - Vertex buffer management
    - Frame synchronization using fences
    - Support for HDR and variable refresh rate displays

## Development Notes

- The project targets Windows 10/11 with minimum feature level D3D_FEATURE_LEVEL_11_0
- Uses Windows SDK version 10.0.26100.0
- Includes Xbox Game Development Kit integration
