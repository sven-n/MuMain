# MinGW-w64 toolchain file for cross-compiling Main on Linux

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

# Compilers
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# Prefer static GCC runtimes to avoid shipping libgcc_s_dw2-1.dll / libstdc++-6.dll
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${CMAKE_SHARED_LINKER_FLAGS_INIT} -static-libgcc -static-libstdc++")

# Only search libraries and headers in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
