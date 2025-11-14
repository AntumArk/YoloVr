#!/bin/bash

# Build script for Linux native compilation

set -e

echo "Building OpenVR Driver for Linux..."

# Create build directory
mkdir -p build-linux
cd build-linux

# Configure with CMake
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release \
      -G "Unix Makefiles" \
      ../driver

# Build the project
echo "Building..."
make -j$(nproc)

echo "Linux build completed!"
echo "Output should be in: output/zincyolotrackers/bin/linux64/"

# List the built files
echo "Built files:"
find ../output -name "*.so" 2>/dev/null || echo "No Linux shared libraries found"