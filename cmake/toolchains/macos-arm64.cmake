# Toolchain for native macOS arm64 builds
#
# This is a NATIVE toolchain (macOS host -> macOS target).
# Native macOS arm64 build using Homebrew LLVM (story 7-6-1).
# Full compilation of game client targets succeeds.
#
# Note: Setting CMAKE_SYSTEM_NAME causes CMAKE_CROSSCOMPILING=TRUE even for
# native builds. This is acceptable — the preset condition restricts to Darwin
# hosts, and no CMake logic in this project checks CMAKE_CROSSCOMPILING.

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)
set(CMAKE_OSX_ARCHITECTURES arm64)

# The oldest macOS the build runs on: the one of this Mac, as major.0. The Homebrew libraries the game links
# (OpenSSL, libjpeg-turbo, Crypto++) are built for it, and Apple's ld warns about each one when the target is older.
# Every Apple Silicon Mac can install the macOS that GitHub's macOS runners use.
if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    execute_process(
        COMMAND sw_vers -productVersion
        OUTPUT_VARIABLE _host_macos_version
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _sw_vers_result
    )
    if(NOT _sw_vers_result EQUAL 0 OR NOT _host_macos_version MATCHES "^([0-9]+)")
        message(FATAL_ERROR "sw_vers -productVersion failed (exit ${_sw_vers_result}): cannot tell this Mac's macOS version")
    endif()
    set(CMAKE_OSX_DEPLOYMENT_TARGET "${CMAKE_MATCH_1}.0")
endif()

# Prefer Homebrew LLVM for macOS builds — Homebrew Clang supports warnings like
# -Wnontrivial-memcall that Apple Clang 17 (Xcode CLI tools) does not.
# Fall back to system clang if Homebrew LLVM is not installed.
if(EXISTS "/opt/homebrew/opt/llvm/bin/clang")
    set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
    set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")
else()
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
endif()

# Detect macOS SDK path at configure time
execute_process(
    COMMAND xcrun --sdk macosx --show-sdk-path
    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _xcrun_result
)
if(NOT _xcrun_result EQUAL 0)
    message(WARNING "xcrun failed (exit ${_xcrun_result}). Install Xcode Command Line Tools: xcode-select --install")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
