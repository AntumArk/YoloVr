cmake_minimum_required(VERSION 3.7.1)

project(zincyolotrackers)

# Require C++14 for std::make_unique and threading support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Position Independent Code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find required packages
# When cross-compiling, help CMake find the MinGW Protobuf installation
if(MINGW)
    set(Protobuf_INCLUDE_DIR /usr/x86_64-w64-mingw32/include)
    set(Protobuf_LIBRARY /usr/x86_64-w64-mingw32/lib/libprotobuf.a)
    set(Protobuf_PROTOC_EXECUTABLE /usr/bin/protoc)
endif()
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

# For your project, this might look something like:
set(OPENVR_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/openvr)

# But we have the binaries we need in this repo, so link those:
# get_filename_component(OPENVR_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}../../.. ABSOLUTE)

# If not set, determine the platform architecture
if(NOT PLATFORM)
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM 64)
  else()
    set(PLATFORM 32)
  endif()
endif()
message(STATUS "Compilation set for ${PLATFORM}bits architectures.")

# OpenVR compatibility checking
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DLINUX -DPOSIX)
  set(ARCH_TARGET linux64)

  if(${PLATFORM} MATCHES 32)
    message(WARNING "OpenVR x86 binaries not provided on GNU/Linux.")
  endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_MACOSX_RPATH 0)
  add_definitions(-DOSX -DPOSIX)
  set(ARCH_TARGET osx32)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_definitions(-D_WIN32)
  set(ARCH_TARGET win${PLATFORM})

  # Binaries path for thirdparties are not generics so we try to guess their suffixes.
  set(WINDOWS_PATH_SUFFIXES win${PLATFORM} Win${PLATFORM} x${PLATFORM})
endif()

find_library(OPENVR_LIBRARIES
  NAMES
    openvr_api
  PATHS
    ${OPENVR_LIB_DIR}/bin
    ${OPENVR_LIB_DIR}/lib
  PATH_SUFFIXES
    osx${PLATFORM}
    linux${PLATFORM}
    win${PLATFORM}
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)

set(OPENVR_INCLUDE_DIR ${OPENVR_LIB_DIR}/headers)

add_subdirectory(${OPENVR_LIB_DIR}/samples/drivers/utils)
# Set output directory to repo root instead of driver/output
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${REPO_ROOT}/output")

set(TARGET_NAME zincyolotrackers)

# This is the name of the driver according to SteamVR
set(DRIVER_NAME "driver_${TARGET_NAME}")

project(${TARGET_NAME})

# Generate protobuf sources from shared proto directory
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${REPO_ROOT}/proto/tracker_data.proto)

add_library(${DRIVER_NAME} SHARED
        src/hmd_driver_factory.cpp
        src/device_provider.h
        src/device_provider.cpp
        src/tracker_device_driver.h
        src/tracker_device_driver.cpp
        src/tracker_data_receiver.h
        src/tracker_data_receiver.cpp
        ${PROTO_SRCS}
        ${PROTO_HDRS}
        )

# This is so we can build directly to "<repo_root>/<target_name>/bin/<platform>/<arch>/<driver_name>.<dll/so>"
set_target_properties(${DRIVER_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/bin/${ARCH_TARGET}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/bin/${ARCH_TARGET}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/bin/${ARCH_TARGET}"
    PREFIX ""
)

target_link_libraries(${DRIVER_NAME} PRIVATE ${OPENVR_LIBRARIES} util_driverlog util_vrmath ${Protobuf_LIBRARIES} Threads::Threads)
target_include_directories(${DRIVER_NAME} PRIVATE ${OPENVR_INCLUDE_DIR} ${Protobuf_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

# Static linking for MinGW to avoid external DLL dependencies
if(MINGW)
    target_link_options(${DRIVER_NAME} PRIVATE -static-libgcc -static-libstdc++ -static)
    # Link Windows socket library for network functionality
    target_link_libraries(${DRIVER_NAME} PRIVATE ws2_32)
endif()

# Copy driver assets to repo root folder structure
add_custom_command(
        TARGET ${DRIVER_NAME}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}
)