# Flow Code: VS0-QUAL-BUILD-DOTNET
# Story 7.8.4: .NET Client Library Native Build
# AC-1: src/CMakeLists.txt uses CMAKE_SYSTEM_NAME for platform-aware DOTNET_RID detection
#
# RED PHASE: Test FAILS until the hardcoded win-x64/win-x86 RID block in
#            src/CMakeLists.txt (lines ~691-699) is replaced with CMAKE_SYSTEM_NAME
#            based detection covering Darwin, Linux, and Windows.
#
# Expected: MuMain/src/CMakeLists.txt contains:
#   - CMAKE_SYSTEM_NAME check for Darwin, Linux, and Windows
#   - DOTNET_RID set to "osx-arm64" (macOS arm64)
#   - DOTNET_RID set to "osx-x64" (macOS x64)
#   - DOTNET_RID set to "linux-x64" (Linux x64)
#   - DOTNET_RID set to "linux-arm64" (Linux arm64) — optional but preferred
#   - Windows fallback retains win-x64 / win-x86 behavior

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CMAKELISTS_FILE)
    set(CMAKELISTS_FILE "${CMAKE_CURRENT_LIST_DIR}/../../src/CMakeLists.txt")
endif()

if(NOT EXISTS "${CMAKELISTS_FILE}")
    message(FATAL_ERROR "AC-1 [7.8.4]: src/CMakeLists.txt does not exist: ${CMAKELISTS_FILE}")
endif()

file(READ "${CMAKELISTS_FILE}" CMAKE_CONTENT)

# --- Check 1: CMAKE_SYSTEM_NAME used for OS-level dispatch ---
string(FIND "${CMAKE_CONTENT}" "CMAKE_SYSTEM_NAME" _pos_system_name)
if(_pos_system_name EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7.8.4]: CMAKE_SYSTEM_NAME not found in src/CMakeLists.txt — required for cross-platform RID detection (Darwin/Linux/Windows)")
endif()

# --- Check 2: Darwin branch exists for macOS ---
string(FIND "${CMAKE_CONTENT}" "Darwin" _pos_darwin)
if(_pos_darwin EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7.8.4]: 'Darwin' not found in src/CMakeLists.txt — required for macOS DOTNET_RID detection")
endif()

# --- Check 3: Linux branch exists ---
string(FIND "${CMAKE_CONTENT}" "Linux" _pos_linux)
if(_pos_linux EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7.8.4]: 'Linux' not found in src/CMakeLists.txt — required for Linux DOTNET_RID detection")
endif()

# --- Check 4: osx-arm64 RID present ---
string(FIND "${CMAKE_CONTENT}" "osx-arm64" _pos_osx_arm64)
if(_pos_osx_arm64 EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7.8.4]: 'osx-arm64' RID not found in src/CMakeLists.txt — required for macOS arm64 native AOT build")
endif()

# --- Check 5: osx-x64 RID present ---
string(FIND "${CMAKE_CONTENT}" "osx-x64" _pos_osx_x64)
if(_pos_osx_x64 EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7.8.4]: 'osx-x64' RID not found in src/CMakeLists.txt — required for macOS x64 native AOT build")
endif()

# --- Check 6: linux-x64 RID present ---
string(FIND "${CMAKE_CONTENT}" "linux-x64" _pos_linux_x64)
if(_pos_linux_x64 EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7.8.4]: 'linux-x64' RID not found in src/CMakeLists.txt — required for Linux x64 native AOT build")
endif()

# --- Check 7: Windows RIDs still present (existing behavior unchanged) ---
string(FIND "${CMAKE_CONTENT}" "win-x64" _pos_win_x64)
if(_pos_win_x64 EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7.8.4]: 'win-x64' RID not found — Windows 64-bit build must still be supported")
endif()

string(FIND "${CMAKE_CONTENT}" "win-x86" _pos_win_x86)
if(_pos_win_x86 EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7.8.4]: 'win-x86' RID not found — Windows 32-bit build must still be supported")
endif()

# --- Check 8: CMAKE_SYSTEM_PROCESSOR used for arm64 vs x64 sub-arch detection ---
string(FIND "${CMAKE_CONTENT}" "CMAKE_SYSTEM_PROCESSOR" _pos_processor)
if(_pos_processor EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7.8.4]: CMAKE_SYSTEM_PROCESSOR not found — required for arm64 vs x64 detection within Darwin/Linux")
endif()

message(STATUS "AC-1 PASS [7.8.4]: src/CMakeLists.txt uses CMAKE_SYSTEM_NAME + CMAKE_SYSTEM_PROCESSOR for cross-platform DOTNET_RID detection (osx-arm64, osx-x64, linux-x64, win-x64, win-x86)")
