# AC-1: Validate FindDotnetAOT.cmake sets MU_DOTNET_RID to a valid platform RID
# AC-5: Validate WSL detection logic is present
#
# Story: 3.1.1 - CMake RID Detection & .NET AOT Build Integration
# RED phase: This test FAILS until cmake/FindDotnetAOT.cmake is created.
#
# Expected: MuMain/cmake/FindDotnetAOT.cmake exists and contains:
#   - RID values for all 5 supported platforms:
#       win-x86, win-x64, osx-arm64, osx-x64, linux-x64
#   - CMAKE_SYSTEM_NAME-based platform detection branches
#   - CMAKE_SIZEOF_VOID_P check for Windows 32/64-bit detection
#   - CMAKE_SYSTEM_PROCESSOR check for macOS arm64 vs x64 detection
#   - WSL detection via /proc/version content match (AC-5)
#   - Flow code VS1-NET-CMAKE-RID in file header

cmake_minimum_required(VERSION 3.25)

# MODULE_FILE is passed via -D from the CTest invocation
if(NOT DEFINED MODULE_FILE)
    # Default relative to this script's location for manual runs
    set(MODULE_FILE "${CMAKE_CURRENT_LIST_DIR}/../../cmake/FindDotnetAOT.cmake")
endif()

# --- Check 1: File exists ---
if(NOT EXISTS "${MODULE_FILE}")
    message(FATAL_ERROR "AC-1: FindDotnetAOT.cmake does not exist: ${MODULE_FILE}")
endif()

file(READ "${MODULE_FILE}" MODULE_CONTENT)

# --- Check 2: All five required RIDs are present ---
foreach(RID "win-x86" "win-x64" "osx-arm64" "osx-x64" "linux-x64")
    string(FIND "${MODULE_CONTENT}" "${RID}" _pos_rid)
    if(_pos_rid EQUAL -1)
        message(FATAL_ERROR "AC-1: Required RID '${RID}' not found in FindDotnetAOT.cmake")
    endif()
endforeach()

# --- Check 3: Platform detection uses CMAKE_SYSTEM_NAME ---
string(FIND "${MODULE_CONTENT}" "CMAKE_SYSTEM_NAME" _pos_system_name)
if(_pos_system_name EQUAL -1)
    message(FATAL_ERROR "AC-1: CMAKE_SYSTEM_NAME not used in FindDotnetAOT.cmake (required for platform detection)")
endif()

# --- Check 4: Windows 32/64-bit detection via CMAKE_SIZEOF_VOID_P ---
string(FIND "${MODULE_CONTENT}" "CMAKE_SIZEOF_VOID_P" _pos_void_p)
if(_pos_void_p EQUAL -1)
    message(FATAL_ERROR "AC-1: CMAKE_SIZEOF_VOID_P not found -- required for win-x86 vs win-x64 detection")
endif()

# --- Check 5: macOS arm64/x64 detection via CMAKE_SYSTEM_PROCESSOR ---
string(FIND "${MODULE_CONTENT}" "CMAKE_SYSTEM_PROCESSOR" _pos_processor)
if(_pos_processor EQUAL -1)
    message(FATAL_ERROR "AC-1: CMAKE_SYSTEM_PROCESSOR not found -- required for osx-arm64 vs osx-x64 detection")
endif()

# --- Check 6: MU_DOTNET_RID variable is set ---
string(REGEX MATCH "set\\(MU_DOTNET_RID" _match_set_rid "${MODULE_CONTENT}")
if(NOT _match_set_rid)
    message(FATAL_ERROR "AC-1: set(MU_DOTNET_RID ...) not found in FindDotnetAOT.cmake")
endif()

# --- Check 7: WSL detection via /proc/version (AC-5) ---
string(FIND "${MODULE_CONTENT}" "/proc/version" _pos_proc_version)
if(_pos_proc_version EQUAL -1)
    message(FATAL_ERROR "AC-5: /proc/version check not found -- required for WSL detection")
endif()

string(REGEX MATCH "[Mm]icrosoft|[Ww][Ss][Ll]" _match_wsl_marker "${MODULE_CONTENT}")
if(NOT _match_wsl_marker)
    message(FATAL_ERROR "AC-5: WSL marker pattern (Microsoft/WSL) not found in /proc/version check")
endif()

# --- Check 8: Flow code traceability ---
string(FIND "${MODULE_CONTENT}" "VS1-NET-CMAKE-RID" _pos_flow_code)
if(_pos_flow_code EQUAL -1)
    message(FATAL_ERROR "AC-1: Flow code 'VS1-NET-CMAKE-RID' not found in FindDotnetAOT.cmake header")
endif()

message(STATUS "AC-1: PASSED -- FindDotnetAOT.cmake contains all required RIDs (win-x86, win-x64, osx-arm64, osx-x64, linux-x64)")
message(STATUS "AC-5: PASSED -- WSL detection logic present (/proc/version + Microsoft/WSL marker)")
