# CMake toolchain file for cross-compiling to Windows using MinGW-w64

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc-posix)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_AR x86_64-w64-mingw32-ar)
set(CMAKE_STRIP x86_64-w64-mingw32-strip)

# Target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Modify default behavior of FIND_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set the platform
set(PLATFORM 64)

# Windows-specific compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -pthread -D_WIN32_WINNT=0x0600")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -pthread -D_WIN32_WINNT=0x0600")

# Enable C++11 threading support for MinGW
add_definitions(-D_GLIBCXX_HAS_GTHREADS)
add_definitions(-DWINVER=0x0600)
add_definitions(-D_WIN32_WINNT=0x0600)

# Enable threading support for MinGW
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Enable static linking to avoid DLL dependencies
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# Set file extensions
set(CMAKE_EXECUTABLE_SUFFIX ".exe")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")