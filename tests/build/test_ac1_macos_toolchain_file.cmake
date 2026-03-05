# AC-1: Validate macOS arm64 toolchain file exists and has correct content
#
# Story: 1.1.1 - Create macOS CMake Toolchain & Presets
# RED phase: This test FAILS until the toolchain file is created.
#
# Expected: MuMain/cmake/toolchains/macos-arm64.cmake exists with:
#   - CMAKE_SYSTEM_NAME Darwin
#   - CMAKE_SYSTEM_PROCESSOR arm64
#   - CMAKE_OSX_ARCHITECTURES arm64
#   - CMAKE_CXX_STANDARD 20
#   - CMAKE_CXX_STANDARD_REQUIRED ON
#   - CMAKE_CXX_EXTENSIONS OFF
#   - Clang compiler configuration (clang / clang++)
#   - macOS SDK detection (xcrun or CMAKE_OSX_SYSROOT)

cmake_minimum_required(VERSION 3.25)

# TOOLCHAIN_FILE is passed via -D from the CTest invocation
if(NOT DEFINED TOOLCHAIN_FILE)
    message(FATAL_ERROR "AC-1: TOOLCHAIN_FILE variable not defined")
endif()

# --- Check 1: File exists ---
if(NOT EXISTS "${TOOLCHAIN_FILE}")
    message(FATAL_ERROR "AC-1: Toolchain file does not exist: ${TOOLCHAIN_FILE}")
endif()

file(READ "${TOOLCHAIN_FILE}" TOOLCHAIN_CONTENT)

# --- Check 2: CMAKE_SYSTEM_NAME Darwin ---
string(FIND "${TOOLCHAIN_CONTENT}" "CMAKE_SYSTEM_NAME" _pos_system_name)
if(_pos_system_name EQUAL -1)
    message(FATAL_ERROR "AC-1: Toolchain file missing CMAKE_SYSTEM_NAME")
endif()
string(REGEX MATCH "set\\(CMAKE_SYSTEM_NAME[ \t]+Darwin\\)" _match_darwin "${TOOLCHAIN_CONTENT}")
if(NOT _match_darwin)
    message(FATAL_ERROR "AC-1: CMAKE_SYSTEM_NAME is not set to Darwin")
endif()

# --- Check 3: CMAKE_SYSTEM_PROCESSOR arm64 ---
string(REGEX MATCH "set\\(CMAKE_SYSTEM_PROCESSOR[ \t]+arm64\\)" _match_proc "${TOOLCHAIN_CONTENT}")
if(NOT _match_proc)
    message(FATAL_ERROR "AC-1: CMAKE_SYSTEM_PROCESSOR is not set to arm64")
endif()

# --- Check 4: CMAKE_OSX_ARCHITECTURES arm64 ---
string(REGEX MATCH "set\\(CMAKE_OSX_ARCHITECTURES[ \t]+arm64\\)" _match_arch "${TOOLCHAIN_CONTENT}")
if(NOT _match_arch)
    message(FATAL_ERROR "AC-1: CMAKE_OSX_ARCHITECTURES is not set to arm64")
endif()

# --- Check 5: Clang compiler (clang / clang++) ---
string(REGEX MATCH "set\\(CMAKE_C_COMPILER[ \t]+clang" _match_cc "${TOOLCHAIN_CONTENT}")
if(NOT _match_cc)
    message(FATAL_ERROR "AC-1: CMAKE_C_COMPILER is not set to clang")
endif()
string(REGEX MATCH "set\\(CMAKE_CXX_COMPILER[ \t]+clang\\+\\+" _match_cxx "${TOOLCHAIN_CONTENT}")
if(NOT _match_cxx)
    message(FATAL_ERROR "AC-1: CMAKE_CXX_COMPILER is not set to clang++")
endif()

# --- Check 6: C++20 standard ---
string(REGEX MATCH "set\\(CMAKE_CXX_STANDARD[ \t]+20\\)" _match_std "${TOOLCHAIN_CONTENT}")
if(NOT _match_std)
    message(FATAL_ERROR "AC-1: CMAKE_CXX_STANDARD is not set to 20")
endif()

# --- Check 7: Extensions OFF ---
string(REGEX MATCH "set\\(CMAKE_CXX_EXTENSIONS[ \t]+OFF\\)" _match_ext "${TOOLCHAIN_CONTENT}")
if(NOT _match_ext)
    message(FATAL_ERROR "AC-1: CMAKE_CXX_EXTENSIONS is not set to OFF")
endif()

# --- Check 8: CMAKE_CXX_STANDARD_REQUIRED ON ---
string(REGEX MATCH "set\\(CMAKE_CXX_STANDARD_REQUIRED[ \t]+ON\\)" _match_req "${TOOLCHAIN_CONTENT}")
if(NOT _match_req)
    message(FATAL_ERROR "AC-1: CMAKE_CXX_STANDARD_REQUIRED is not set to ON")
endif()

# --- Check 9: macOS SDK detection (xcrun or CMAKE_OSX_SYSROOT) ---
string(FIND "${TOOLCHAIN_CONTENT}" "xcrun" _pos_xcrun)
string(FIND "${TOOLCHAIN_CONTENT}" "CMAKE_OSX_SYSROOT" _pos_sysroot)
if(_pos_xcrun EQUAL -1 AND _pos_sysroot EQUAL -1)
    message(FATAL_ERROR "AC-1: Toolchain file missing macOS SDK detection (xcrun or CMAKE_OSX_SYSROOT)")
endif()

# --- Check 10: NO cross-compile artifacts that should not be present ---
string(FIND "${TOOLCHAIN_CONTENT}" "CMAKE_FIND_ROOT_PATH_MODE" _pos_root_mode)
if(NOT _pos_root_mode EQUAL -1)
    message(FATAL_ERROR "AC-1: Toolchain contains CMAKE_FIND_ROOT_PATH_MODE_* (cross-compile artifact -- not allowed for native macOS build)")
endif()

string(FIND "${TOOLCHAIN_CONTENT}" "static-libgcc" _pos_static)
if(NOT _pos_static EQUAL -1)
    message(FATAL_ERROR "AC-1: Toolchain contains static-libgcc flag (MinGW-specific -- not allowed for native macOS build)")
endif()

message(STATUS "AC-1: PASSED -- macOS arm64 toolchain file is valid")
