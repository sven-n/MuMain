# Toolchain for native macOS arm64 builds
#
# This is a NATIVE toolchain (macOS host -> macOS target).
# Configure-only until SDL3 migration removes Win32/DirectX API dependencies.
# Full compile will be possible after Phase 1-2 of the cross-platform plan.
#
# Note: Setting CMAKE_SYSTEM_NAME causes CMAKE_CROSSCOMPILING=TRUE even for
# native builds. This is acceptable — the preset condition restricts to Darwin
# hosts, and no CMake logic in this project checks CMAKE_CROSSCOMPILING.

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)
set(CMAKE_OSX_ARCHITECTURES arm64)

# Clang is the native macOS compiler
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Detect macOS SDK path at configure time
execute_process(
    COMMAND xcrun --sdk macosx --show-sdk-path
    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
