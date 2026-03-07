# AC-6: Validate graceful failure when dotnet is not found
#
# Story: 3.1.1 - CMake RID Detection & .NET AOT Build Integration
# RED phase: This test FAILS until cmake/FindDotnetAOT.cmake implements
#            graceful failure with a non-fatal WARNING message.
#
# Expected: MuMain/cmake/FindDotnetAOT.cmake contains:
#   - find_program(DOTNET_EXECUTABLE dotnet ...) call
#   - message(WARNING ...) emitted when dotnet not found (NOT FATAL_ERROR)
#   - set(DOTNETAOT_FOUND FALSE) when dotnet not found
#   - The warning message contains the required prefix: "PLAT: FindDotnetAOT"
#   - A return() or equivalent guard so build proceeds without dotnet
#
# Expected: MuMain/CMakeLists.txt contains:
#   - DOTNETAOT_FOUND guard around BuildDotNetAOT target
#   - add_compile_definitions(MU_DOTNET_LIB_EXT=...) called even when
#     DOTNETAOT_FOUND=FALSE (CI builds must compile without dotnet)

cmake_minimum_required(VERSION 3.25)

# MODULE_FILE and CMAKELISTS_FILE are passed via -D from CTest invocation
if(NOT DEFINED MODULE_FILE)
    set(MODULE_FILE "${CMAKE_CURRENT_LIST_DIR}/../../cmake/FindDotnetAOT.cmake")
endif()
if(NOT DEFINED CMAKELISTS_FILE)
    set(CMAKELISTS_FILE "${CMAKE_CURRENT_LIST_DIR}/../../CMakeLists.txt")
endif()

# --- Check 1: FindDotnetAOT.cmake exists ---
if(NOT EXISTS "${MODULE_FILE}")
    message(FATAL_ERROR "AC-6: FindDotnetAOT.cmake does not exist: ${MODULE_FILE}")
endif()

file(READ "${MODULE_FILE}" MODULE_CONTENT)

# --- Check 2: find_program(DOTNETAOT_EXECUTABLE ...) is used ---
string(REGEX MATCH "find_program[\\( \t]*DOTNETAOT_EXECUTABLE" _match_find_prog "${MODULE_CONTENT}")
if(NOT _match_find_prog)
    message(FATAL_ERROR "AC-6: find_program(DOTNETAOT_EXECUTABLE ...) not found in FindDotnetAOT.cmake")
endif()

# --- Check 3: message(WARNING ...) for graceful failure (NOT FATAL_ERROR) ---
string(REGEX MATCH "message\\(WARNING" _match_warning "${MODULE_CONTENT}")
if(NOT _match_warning)
    message(FATAL_ERROR "AC-6: message(WARNING ...) not found in FindDotnetAOT.cmake -- dotnet-not-found must use WARNING not FATAL_ERROR")
endif()

# Ensure there is no message(FATAL_ERROR) for dotnet-not-found path
# (FATAL_ERROR is acceptable for truly unexpected errors, but not for missing dotnet)
string(REGEX MATCHALL "message\\(FATAL_ERROR[^\n]*dotnet[^\n]*\\)" _fatal_dotnet_msgs "${MODULE_CONTENT}")
list(LENGTH _fatal_dotnet_msgs _fatal_count)
if(_fatal_count GREATER 0)
    message(FATAL_ERROR "AC-6: Found message(FATAL_ERROR ... dotnet ...) -- missing dotnet must be a WARNING, not a fatal error")
endif()

# --- Check 4: Required warning message prefix ---
string(FIND "${MODULE_CONTENT}" "PLAT: FindDotnetAOT" _pos_plat_prefix)
if(_pos_plat_prefix EQUAL -1)
    message(FATAL_ERROR "AC-6: Warning message prefix 'PLAT: FindDotnetAOT' not found -- AC-STD-5 requires this exact prefix")
endif()

# --- Check 5: DOTNETAOT_FOUND is set to FALSE on failure path ---
string(REGEX MATCH "set\\(DOTNETAOT_FOUND[ \t]+FALSE\\)" _match_found_false "${MODULE_CONTENT}")
if(NOT _match_found_false)
    message(FATAL_ERROR "AC-6: set(DOTNETAOT_FOUND FALSE) not found in FindDotnetAOT.cmake -- required for downstream if(DOTNETAOT_FOUND) guards")
endif()

# --- Check 6: DOTNETAOT_FOUND is also set to TRUE on success path ---
string(REGEX MATCH "set\\(DOTNETAOT_FOUND[ \t]+TRUE\\)" _match_found_true "${MODULE_CONTENT}")
if(NOT _match_found_true)
    message(FATAL_ERROR "AC-6: set(DOTNETAOT_FOUND TRUE) not found in FindDotnetAOT.cmake -- required for success path")
endif()

# --- Check 7: CMakeLists.txt guards BuildDotNetAOT behind DOTNETAOT_FOUND ---
if(NOT EXISTS "${CMAKELISTS_FILE}")
    message(FATAL_ERROR "AC-6: CMakeLists.txt does not exist: ${CMAKELISTS_FILE}")
endif()

file(READ "${CMAKELISTS_FILE}" CMAKE_CONTENT)

string(FIND "${CMAKE_CONTENT}" "DOTNETAOT_FOUND" _pos_found_guard)
if(_pos_found_guard EQUAL -1)
    message(FATAL_ERROR "AC-6: DOTNETAOT_FOUND not referenced in CMakeLists.txt -- BuildDotNetAOT must be guarded by if(DOTNETAOT_FOUND)")
endif()

# --- Check 8: add_compile_definitions is not gated by DOTNETAOT_FOUND alone ---
# MU_DOTNET_LIB_EXT must be passed even when dotnet is not found
# Verify the else() branch or unconditional add_compile_definitions exists
string(FIND "${CMAKE_CONTENT}" "add_compile_definitions" _pos_add_def)
if(_pos_add_def EQUAL -1)
    message(FATAL_ERROR "AC-6: add_compile_definitions not found in CMakeLists.txt -- MU_DOTNET_LIB_EXT must be defined for CI builds even without dotnet")
endif()

message(STATUS "AC-6: PASSED -- FindDotnetAOT.cmake emits WARNING (not FATAL_ERROR) when dotnet not found, sets DOTNETAOT_FOUND=FALSE, and uses 'PLAT: FindDotnetAOT' prefix")
