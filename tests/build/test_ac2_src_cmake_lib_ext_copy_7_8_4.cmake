# Flow Code: VS0-QUAL-BUILD-DOTNET
# Story 7.8.4: .NET Client Library Native Build
# AC-2: src/CMakeLists.txt copy_if_different command uses MU_DOTNET_LIB_EXT
#        instead of hardcoded .dll extension
#
# RED PHASE: Test FAILS until line ~712 in src/CMakeLists.txt replaces the
#            hardcoded "MUnique.Client.Library.dll" copy source with a
#            platform-variable-based filename.
#
# Expected: MuMain/src/CMakeLists.txt:
#   - Does NOT use hardcoded "MUnique.Client.Library.dll" in copy_if_different
#   - Uses MU_DOTNET_LIB_EXT or equivalent cmake variable for the extension
#   - DOTNET_DLL_PATH variable accounts for platform extension

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CMAKELISTS_FILE)
    set(CMAKELISTS_FILE "${CMAKE_CURRENT_LIST_DIR}/../../src/CMakeLists.txt")
endif()

if(NOT EXISTS "${CMAKELISTS_FILE}")
    message(FATAL_ERROR "AC-2 [7.8.4]: src/CMakeLists.txt does not exist: ${CMAKELISTS_FILE}")
endif()

file(READ "${CMAKELISTS_FILE}" CMAKE_CONTENT)

# --- Check 1: Hardcoded .dll copy path must be gone ---
# The old pattern: copy_if_different "...MUnique.Client.Library.dll"
# This check ensures the hard-coded extension is replaced with a variable.
string(REGEX MATCH "copy_if_different[^\n]*MUnique\\.Client\\.Library\\.dll" _match_hardcoded "${CMAKE_CONTENT}")
if(_match_hardcoded)
    message(FATAL_ERROR "AC-2 FAIL [7.8.4]: Hardcoded 'MUnique.Client.Library.dll' still present in copy_if_different command — must use MU_DOTNET_LIB_EXT or platform variable instead")
endif()

# --- Check 2: MU_DOTNET_LIB_EXT variable referenced in CMakeLists.txt ---
string(FIND "${CMAKE_CONTENT}" "MU_DOTNET_LIB_EXT" _pos_ext_var)
if(_pos_ext_var EQUAL -1)
    message(FATAL_ERROR "AC-2 FAIL [7.8.4]: MU_DOTNET_LIB_EXT not referenced in src/CMakeLists.txt — must be used for platform-correct library extension")
endif()

# --- Check 3: .dylib and .so are referenced (platform extensions set) ---
string(FIND "${CMAKE_CONTENT}" ".dylib" _pos_dylib)
if(_pos_dylib EQUAL -1)
    message(FATAL_ERROR "AC-2 FAIL [7.8.4]: '.dylib' extension not found in src/CMakeLists.txt — required for macOS native AOT output")
endif()

string(FIND "${CMAKE_CONTENT}" ".so" _pos_so)
if(_pos_so EQUAL -1)
    message(FATAL_ERROR "AC-2 FAIL [7.8.4]: '.so' extension not found in src/CMakeLists.txt — required for Linux native AOT output")
endif()

# --- Check 4: DOTNET_DLL_PATH uses the extension variable ---
# The path variable should not hardcode .dll
string(REGEX MATCH "set\\(DOTNET_DLL_PATH[^\n]*\\.dll[^\n]*\\)" _match_dll_path "${CMAKE_CONTENT}")
if(_match_dll_path)
    message(FATAL_ERROR "AC-2 FAIL [7.8.4]: DOTNET_DLL_PATH still hardcodes '.dll' — must use MU_DOTNET_LIB_EXT variable for platform-correct extension")
endif()

message(STATUS "AC-2 PASS [7.8.4]: src/CMakeLists.txt copy command uses MU_DOTNET_LIB_EXT (.dll/.dylib/.so) — no hardcoded .dll extension in copy_if_different")
