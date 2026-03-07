# AC-2: Validate MU_DOTNET_LIB_EXT is set per platform and passed to C++ compiler
#
# Story: 3.1.1 - CMake RID Detection & .NET AOT Build Integration
# RED phase: This test FAILS until cmake/FindDotnetAOT.cmake and CMakeLists.txt
#            are updated with MU_DOTNET_LIB_EXT support.
#
# Expected: MuMain/cmake/FindDotnetAOT.cmake contains:
#   - set(MU_DOTNET_LIB_EXT ".dll") for Windows
#   - set(MU_DOTNET_LIB_EXT ".dylib") for macOS/Darwin
#   - set(MU_DOTNET_LIB_EXT ".so") for Linux
#
# Expected: MuMain/CMakeLists.txt contains:
#   - add_compile_definitions(MU_DOTNET_LIB_EXT=...) to pass the define to C++ compiler
#   - This must be present even when DOTNETAOT_FOUND=FALSE (fallback for CI builds)

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
    message(FATAL_ERROR "AC-2: FindDotnetAOT.cmake does not exist: ${MODULE_FILE}")
endif()

file(READ "${MODULE_FILE}" MODULE_CONTENT)

# --- Check 2: .dll extension for Windows ---
string(FIND "${MODULE_CONTENT}" ".dll" _pos_dll)
if(_pos_dll EQUAL -1)
    message(FATAL_ERROR "AC-2: '.dll' extension not found in FindDotnetAOT.cmake (required for Windows)")
endif()

# --- Check 3: .dylib extension for macOS ---
string(FIND "${MODULE_CONTENT}" ".dylib" _pos_dylib)
if(_pos_dylib EQUAL -1)
    message(FATAL_ERROR "AC-2: '.dylib' extension not found in FindDotnetAOT.cmake (required for macOS)")
endif()

# --- Check 4: .so extension for Linux ---
string(FIND "${MODULE_CONTENT}" ".so" _pos_so)
if(_pos_so EQUAL -1)
    message(FATAL_ERROR "AC-2: '.so' extension not found in FindDotnetAOT.cmake (required for Linux)")
endif()

# --- Check 5: MU_DOTNET_LIB_EXT variable is set ---
string(REGEX MATCH "set\\(MU_DOTNET_LIB_EXT" _match_set_ext "${MODULE_CONTENT}")
if(NOT _match_set_ext)
    message(FATAL_ERROR "AC-2: set(MU_DOTNET_LIB_EXT ...) not found in FindDotnetAOT.cmake")
endif()

# --- Check 6: CMakeLists.txt passes MU_DOTNET_LIB_EXT to C++ compiler ---
if(NOT EXISTS "${CMAKELISTS_FILE}")
    message(FATAL_ERROR "AC-2: CMakeLists.txt does not exist: ${CMAKELISTS_FILE}")
endif()

file(READ "${CMAKELISTS_FILE}" CMAKE_CONTENT)

string(FIND "${CMAKE_CONTENT}" "MU_DOTNET_LIB_EXT" _pos_define)
if(_pos_define EQUAL -1)
    message(FATAL_ERROR "AC-2: MU_DOTNET_LIB_EXT not referenced in CMakeLists.txt -- must be passed via add_compile_definitions")
endif()

string(FIND "${CMAKE_CONTENT}" "add_compile_definitions" _pos_add_compile)
if(_pos_add_compile EQUAL -1)
    message(FATAL_ERROR "AC-2: add_compile_definitions not found in CMakeLists.txt -- MU_DOTNET_LIB_EXT must be passed to C++ compiler")
endif()

# --- Check 7: MU_ENABLE_DOTNET option exists in CMakeLists.txt ---
string(REGEX MATCH "option\\(MU_ENABLE_DOTNET" _match_option "${CMAKE_CONTENT}")
if(NOT _match_option)
    message(FATAL_ERROR "AC-2: option(MU_ENABLE_DOTNET ...) not found in CMakeLists.txt")
endif()

# --- Check 8: FindDotnetAOT is included/integrated in CMakeLists.txt ---
string(FIND "${CMAKE_CONTENT}" "FindDotnetAOT" _pos_include)
if(_pos_include EQUAL -1)
    message(FATAL_ERROR "AC-2: FindDotnetAOT not referenced in CMakeLists.txt -- include(FindDotnetAOT) required")
endif()

message(STATUS "AC-2: PASSED -- MU_DOTNET_LIB_EXT is set for all platforms (.dll/.dylib/.so) and passed to C++ compiler via add_compile_definitions")
