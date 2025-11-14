# Docker Development Environment - Package Guide

This document explains all the packages installed in our Docker containers and why they're needed for OpenVR driver development.

## Linux Builder Container (`Dockerfile.linux-builder`)

### Core Build Tools
- **`build-essential`**: Meta-package that installs essential compilation tools
  - `gcc`: GNU C Compiler
  - `g++`: GNU C++ Compiler  
  - `make`: Build automation tool
  - `libc6-dev`: C library development files
  - `dpkg-dev`: Debian package development tools

- **`cmake`**: Cross-platform build system generator
  - Generates build files (Makefiles, Ninja files, etc.)
  - Required by OpenVR and our project configuration

- **`ninja-build`**: Fast parallel build system
  - Alternative to `make` with better parallelization
  - Speeds up compilation on multi-core systems

- **`git`**: Version control system
  - Used for cloning dependencies and source code management

- **`pkg-config`**: Library compilation helper
  - Helps find libraries and their compilation flags
  - Essential for linking against system libraries

### Graphics Libraries (OpenGL)
- **`libgl1-mesa-dev`**: OpenGL development libraries
  - Mesa is an open-source implementation of OpenGL
  - Provides headers and libraries for OpenGL development
  - Required for VR applications that use OpenGL rendering

- **`libglu1-mesa-dev`**: OpenGL Utility Library
  - Higher-level OpenGL functions and utilities
  - Common in graphics applications

### X11 Window System Libraries
These libraries are essential for OpenVR on Linux, as VR applications need to interact with the display system:

- **`libx11-dev`**: Core X11 library
  - Basic X Window System functionality
  - Window creation, event handling, drawing primitives

- **`libxrandr-dev`**: X11 RandR extension
  - Screen resolution and rotation management
  - Multi-monitor configuration
  - Essential for VR headset display management

- **`libxinerama-dev`**: X11 Xinerama extension  
  - Multi-monitor support for treating multiple screens as one
  - Used by VR systems for extended desktop setups

- **`libxcursor-dev`**: X11 cursor management
  - Mouse cursor handling and customization
  - VR applications may need to hide/modify cursors

- **`libxi-dev`**: X11 Input extension
  - Advanced input device handling (mouse, keyboard, touchscreens)
  - VR controllers and input devices integration

- **`libxss-dev`**: X11 Screen Saver extension
  - Screen saver management and prevention
  - VR applications often disable screen savers during use

## Windows Builder Container (`Dockerfile.windows-builder`)

### Core Build Tools
Same as Linux builder for host compilation:
- **`build-essential`**, **`cmake`**, **`ninja-build`**, **`git`**, **`pkg-config`**

### MinGW-w64 Cross-Compilation Toolchain
MinGW-w64 (Minimalist GNU for Windows) allows compiling Windows applications from Linux:

- **`gcc-mingw-w64-x86-64`**: MinGW-w64 C compiler for 64-bit Windows
  - Cross-compiler that produces Windows PE executables
  - Targets x86_64 (64-bit) Windows architecture

- **`g++-mingw-w64-x86-64`**: MinGW-w64 C++ compiler for 64-bit Windows
  - C++ cross-compiler with Windows-specific adaptations
  - Supports modern C++ standards (C++11, C++14, C++17)

- **`mingw-w64-tools`**: Additional MinGW-w64 utilities
  - `windres`: Windows resource compiler (for .rc files)
  - `dlltool`: DLL import library generator
  - `strip`: Binary symbol stripper

### Threading Model Configuration
```bash
update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix
update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix
```

**Why POSIX threading?**
- MinGW-w64 offers two threading models: `win32` and `posix`
- **win32**: Uses Windows native threading APIs (limited C++11 support)
- **posix**: Uses POSIX threading (full C++11/14 `std::thread` support)
- Our code uses `std::thread`, so we need the POSIX model

### Environment Variables
```dockerfile
ENV CC=x86_64-w64-mingw32-gcc-posix
ENV CXX=x86_64-w64-mingw32-g++-posix  
ENV AR=x86_64-w64-mingw32-ar
ENV STRIP=x86_64-w64-mingw32-strip
```

These environment variables tell the build system which cross-compilation tools to use:
- **CC/CXX**: C/C++ compilers for Windows targets
- **AR**: Archive tool for creating static libraries (.a files)
- **STRIP**: Tool for removing debug symbols to reduce binary size

## Why Docker for Development?

### Clean Environment
- No need to install cross-compilation tools on host system
- Avoids conflicts between different project requirements
- Consistent build environment across different developer machines

### Reproducible Builds  
- Same Ubuntu base image for all developers
- Locked package versions ensure consistent results
- Build caches speed up repeated builds

### Cross-Platform Support
- Build Windows binaries from Linux/macOS
- No need for Windows development machine
- CI/CD systems can build all targets from Linux runners

### Isolation
- Dependencies are contained within containers
- Host system stays clean and unmodified
- Easy to update or change build environments

## Troubleshooting Common Issues

### Missing OpenVR Libraries
If you get linking errors, ensure these files exist:
- **Linux**: `driver/lib/openvr/lib/linux64/libopenvr_api.so`
- **Windows**: `driver/lib/openvr/lib/win64/openvr_api.lib` (MSVC) or `.dll.a` (MinGW)

### Threading Errors on Windows
If you see `std::thread` errors during Windows builds:
- Ensure MinGW is using POSIX threading model
- Check that the toolchain file sets appropriate compiler flags
- Verify C++14 standard is enabled (required for `std::make_unique`)

### X11 Errors on Linux  
If you get X11-related errors:
- Ensure all X11 development packages are installed
- Check that OpenGL libraries are available
- Verify the container has access to display libraries