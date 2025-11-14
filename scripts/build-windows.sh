#!/bin/bash

# Build script for Windows cross-compilation

set -e

echo "Building OpenVR Driver for Windows..."

# Create build directory
mkdir -p build-windows
cd build-windows

# Configure with CMake
echo "Configuring with CMake..."
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/windows-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -G "Unix Makefiles" \
      ../driver

# Build the project
echo "Building..."
make -j$(nproc)

echo "Windows build completed!"
echo "Output should be in: output/zincyolotrackers/bin/win64/"

# List the built files
echo "Built files:"
find ../output -name "*.dll" -o -name "*.exe" 2>/dev/null || echo "No Windows binaries found"