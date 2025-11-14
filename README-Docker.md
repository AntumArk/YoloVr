# Docker Cross-Compilation Setup

This directory contains Docker configuration for cross-compiling the OpenVR driver for Windows.

## Files Created:
- `docker-compose.yml` - Docker Compose configuration
- `Dockerfile.windows-builder` - Windows cross-compilation environment
- `cmake/windows-toolchain.cmake` - CMake toolchain for MinGW-w64
- `scripts/build-windows.sh` - Build script

## Usage:

### Quick Build:
```bash
# Build for Windows using Docker
docker-compose up windows-builder
```

### Interactive Development:
```bash
# Start interactive container for development
docker-compose run windows-dev
```

### Manual Build in Container:
```bash
# Run interactive container
docker-compose run windows-dev

# Inside container, run:
./scripts/build-windows.sh
```

## Output:
Windows binaries will be created in:
```
output/zincyolotrackers/bin/win64/driver_zincyolotrackers.dll
```

## Requirements:
- Docker and Docker Compose installed
- OpenVR Windows binaries in `driver/lib/openvr/bin/win64/`

## Notes:
- The build uses MinGW-w64 for cross-compilation
- Static linking is enabled to minimize dependencies
- Build cache is stored in a Docker volume for faster subsequent builds

## Troubleshooting:
If you get OpenVR linking errors, ensure Windows OpenVR binaries are present:
- `driver/lib/openvr/lib/win64/openvr_api.lib` (or .dll.a for MinGW)
- `driver/lib/openvr/bin/win64/openvr_api.dll`