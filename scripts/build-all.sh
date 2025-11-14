#!/bin/bash

# Build script for both Linux and Windows platforms

set -e

echo "🚀 Building OpenVR Driver for multiple platforms..."
echo ""

# Build for Linux
echo "📦 Building for Linux..."
docker-compose up linux-builder
echo "✅ Linux build completed!"
echo ""

# Build for Windows
echo "📦 Building for Windows..."
docker-compose up windows-builder  
echo "✅ Windows build completed!"
echo ""

echo "🎉 All builds completed successfully!"
echo ""
echo "📋 Build artifacts:"
find output -name "*.so" -o -name "*.dll" | while read file; do
    echo "  - $(file "$file" | cut -d: -f2-)"
done

echo ""
echo "📁 Output structure:"
tree output/ 2>/dev/null || find output -type f | sort