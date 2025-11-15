cmake_minimum_required(VERSION 3.7.1)

project(zincyolotrackers)

# Require C++14 for std::make_unique and threading support
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

add_library(${DRIVER_NAME} SHARED
        src/hmd_driver_factory.cpp
        src/device_provider.h
        src/device_provider.cpp
        src/tracker_device_driver.h
        src/tracker_device_driver.cpp
        )

# This is so we can build directly to "<repo_root>/<target_name>/bin/<platform>/<arch>/<driver_name>.<dll/so>"
set_target_properties(${DRIVER_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/bin/${ARCH_TARGET}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/bin/${ARCH_TARGET}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/bin/${ARCH_TARGET}"
    PREFIX ""
)

target_link_libraries(${DRIVER_NAME} PRIVATE ${OPENVR_LIBRARIES} util_driverlog util_vrmath)
target_include_directories(${DRIVER_NAME} PRIVATE ${OPENVR_INCLUDE_DIR})

# Find and link threading library (especially important for MinGW)
find_package(Threads REQUIRED)
target_link_libraries(${DRIVER_NAME} PRIVATE Threads::Threads)

# Copy driver assets to repo root folder structure
add_custom_command(
        TARGET ${DRIVER_NAME}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}
)

# Copy openvr_api.dll to the driver bin directory
add_custom_command(
        TARGET ${DRIVER_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OPENVR_LIB_DIR}/bin/${ARCH_TARGET}/openvr_api.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/bin/${ARCH_TARGET}/openvr_api.dll
)

# Copy MinGW runtime DLLs when cross-compiling for Windows
if(MINGW)
    # Find MinGW compiler directory and lib paths
    get_filename_component(MINGW_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    
    # Find gcc version directory for libgcc and libstdc++
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libgcc_s_seh-1.dll
        OUTPUT_VARIABLE LIBGCC_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    get_filename_component(MINGW_GCC_DIR ${LIBGCC_PATH} DIRECTORY)
    
    # Find libwinpthread location
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libwinpthread-1.dll
        OUTPUT_VARIABLE LIBWINPTHREAD_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    # Copy runtime DLLs
    if(EXISTS "${LIBGCC_PATH}")
        add_custom_command(
            TARGET ${DRIVER_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LIBGCC_PATH}"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/bin/${ARCH_TARGET}/libgcc_s_seh-1.dll"
        )
    endif()
    
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++-6.dll
        OUTPUT_VARIABLE LIBSTDCPP_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(EXISTS "${LIBSTDCPP_PATH}")
        add_custom_command(
            TARGET ${DRIVER_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LIBSTDCPP_PATH}"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/bin/${ARCH_TARGET}/libstdc++-6.dll"
        )
    endif()
    
    if(EXISTS "${LIBWINPTHREAD_PATH}")
        add_custom_command(
            TARGET ${DRIVER_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LIBWINPTHREAD_PATH}"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}/bin/${ARCH_TARGET}/libwinpthread-1.dll"
        )
    endif()
endif()