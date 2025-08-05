# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**yangine** is a DirectX 12-based graphics engine written in C++20. It's a Windows-specific project that builds a static library (`yangine.lib`) and includes a sample application demonstrating triangle rendering.

## Build System

### Commands

- **Build library**: `make build` - Compiles all source files and creates `yangine.lib` in the `build/` directory
- **Build application**: `make build_app` - Builds the library then compiles the MSBuild solution (requires SOLUTION FILE to be specified)
- **Clean**: `make clean` - Removes the entire `build/` directory
- **Format code**: `make lint` - Runs `clang-format` on all `.cpp` and `.h` files

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

1. **Application** (`src/Application.h/cpp`): Main application class that manages initialization, message loop, and shutdown
2. **Renderer** (`src/Renderer.h/cpp`): DirectX 12 renderer implementing `DX::IDeviceNotify` interface, handles triangle rendering
3. **DeviceResources** (`src/DeviceResources.h/cpp`): Wrapper for Direct3D 12 device, command queue, swap chain, and resource management
4. **WindowManager** (`src/WindowManager.h/cpp`): Handles Win32 window creation and management

### Key Patterns

- Uses RAII and smart pointers (`std::unique_ptr`, `Microsoft::WRL::ComPtr`)
- Implements device lost/restored pattern through `IDeviceNotify` interface
- Follows DirectX 12 best practices with command allocators per back buffer
- Uses precompiled headers (`pch.h`) for performance

### Directory Structure

- `src/`: Core engine source files
- `include/`: Public headers (currently contains placeholder `yangine.h`)
- `vendor/`: Third-party libraries (DirectX headers, WinPixEvent)
- `build/`: Generated build artifacts
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