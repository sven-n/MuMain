# Toolchain for native Linux x86_64 builds
#
# This is a NATIVE toolchain (Linux host -> Linux target).
# Unlike the MinGW cross-compile toolchain, this does NOT set
# cross-compilation root path modes or static runtime flags.
#
# Note: Setting CMAKE_SYSTEM_NAME causes CMAKE_CROSSCOMPILING=TRUE even for
# native builds. This is acceptable — the preset condition restricts to Linux
# hosts, and no CMake logic in this project checks CMAKE_CROSSCOMPILING.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# GCC ships with standard Linux distros (requires GCC 10+ for C++20)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
