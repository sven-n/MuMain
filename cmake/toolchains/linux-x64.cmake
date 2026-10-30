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

# Build for the x86-64 baseline, which every 64-bit x86 CPU runs (Windows
# builds the same way). Without -march, GCC uses its own default, and that
# differs: GitHub's ubuntu-26.04 runners install Ubuntu's amd64v3 packages,
# whose GCC defaults to x86-64-v3 (AVX2 and FMA). Main would then need an AVX2
# CPU, and GCC would fuse multiplies and adds wherever it chooses, so the same
# float arithmetic in two places can round differently (the bit-for-bit
# animation test fails).
set(CMAKE_C_FLAGS_INIT "-march=x86-64")
set(CMAKE_CXX_FLAGS_INIT "-march=x86-64")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
