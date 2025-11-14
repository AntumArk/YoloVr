# Docker Cross-Platform Compilation Setup

This directory contains Docker configuration for compiling the OpenVR driver for both Linux and Windows platforms in isolated containers.

## Files Created:
- `docker-compose.yml` - Docker Compose configuration for both platforms
- `Dockerfile.linux-builder` - Native Linux compilation environment
- `Dockerfile.windows-builder` - Windows cross-compilation environment  
- `cmake/windows-toolchain.cmake` - CMake toolchain for MinGW-w64
- `scripts/build-linux.sh` - Linux build script
- `scripts/build-windows.sh` - Windows build script

## Usage:

### Linux Native Build:
```bash
# Quick Linux build
docker-compose up linux-builder

# Interactive Linux development
docker-compose run linux-dev
```

### Windows Cross-Compilation:
```bash
# Quick Windows build
docker-compose up windows-builder

# Interactive Windows development
docker-compose run windows-dev
```

### Manual Build in Container:
```bash
# Linux build in container
docker-compose run linux-dev
./scripts/build-linux.sh

# Windows build in container  
docker-compose run windows-dev
./scripts/build-windows.sh
```

### Build Both Platforms:
```bash
# Build for both Linux and Windows
docker-compose up linux-builder windows-builder

# Or use the convenient script
./scripts/build-all.sh
```

### Clean Builds:
```bash
# Remove build caches and rebuild everything
docker-compose down -v
./scripts/build-all.sh
```

## Output:
Binaries will be created in:
```
output/zincyolotrackers/bin/linux64/driver_zincyolotrackers.so    # Linux
output/zincyolotrackers/bin/win64/libdriver_zincyolotrackers.dll  # Windows
```

## Requirements:
- Docker and Docker Compose installed
- OpenVR binaries in `driver/lib/openvr/bin/` for respective platforms

## Benefits:
- **Clean Development Environment**: No need to install build tools on host
- **Consistent Builds**: Same environment for all developers
- **Cross-Platform**: Build for Windows from Linux (or any platform)
- **Isolated Dependencies**: No conflicts with host system libraries
- **Cached Builds**: Docker volumes speed up subsequent builds

## Notes:
- Linux build uses native GCC compilation
- Windows build uses MinGW-w64 with POSIX threading support
- Both builds require C++14 for std::make_unique support
- Build caches are stored in Docker volumes for faster rebuilds

## Troubleshooting:
If you get OpenVR linking errors, ensure the required binaries are present:
- Linux: `driver/lib/openvr/lib/linux64/libopenvr_api.so`
- Windows: `driver/lib/openvr/lib/win64/openvr_api.lib` (or .dll.a for MinGW)