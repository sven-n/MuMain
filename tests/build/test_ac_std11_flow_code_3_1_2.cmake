# AC-STD-11: Flow Code Traceability — VS1-NET-CONNECTION-XPLAT
# AC-STD-3 / AC-4: Zero platform ifdefs in Connection.h
# AC-VAL-3: Verify VS1-NET-CONNECTION-XPLAT present + no Win32/POSIX direct API calls
#
# Story: 3.1.2 - Connection.h Cross-Platform Updates
# RED phase: This test FAILS until Connection.h is refactored to:
#   - Add "// Flow Code: VS1-NET-CONNECTION-XPLAT" in header comment
#   - Remove all #ifdef _WIN32 blocks
#   - Remove LoadLibrary calls (replaced by mu::platform::Load())
#   - Remove dlopen calls (replaced by mu::platform::Load())
#
# Run manually: cmake -P tests/build/test_ac_std11_flow_code_3_1_2.cmake

cmake_minimum_required(VERSION 3.25)

# CONNECTION_H is passed via -D from CTest invocation, or defaults to standard path
if(NOT DEFINED CONNECTION_H)
    set(CONNECTION_H "${CMAKE_CURRENT_LIST_DIR}/../../src/source/Dotnet/Connection.h")
endif()

# --- Check 1: File exists ---
if(NOT EXISTS "${CONNECTION_H}")
    message(FATAL_ERROR "AC-STD-11: Connection.h not found at ${CONNECTION_H}")
endif()

file(READ "${CONNECTION_H}" CONNECTION_H_CONTENT)

# --- Check 2: Flow code VS1-NET-CONNECTION-XPLAT is present (AC-STD-11) ---
string(FIND "${CONNECTION_H_CONTENT}" "VS1-NET-CONNECTION-XPLAT" _pos_flow_code)
if(_pos_flow_code EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-CONNECTION-XPLAT' not found in Connection.h -- add '// Flow Code: VS1-NET-CONNECTION-XPLAT' to the file header")
endif()

# --- Check 3: Flow code appears in header (first 2000 chars) ---
string(SUBSTRING "${CONNECTION_H_CONTENT}" 0 2000 _header_block)
string(FIND "${_header_block}" "VS1-NET-CONNECTION-XPLAT" _pos_in_header)
if(_pos_in_header EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-CONNECTION-XPLAT' not found in the first ~2000 chars of Connection.h -- it must be in the file header comment, not buried in logic")
endif()

# --- Check 4: No #ifdef _WIN32 (AC-4 / AC-STD-3) ---
string(FIND "${CONNECTION_H_CONTENT}" "#ifdef _WIN32" _pos_ifdef)
if(NOT _pos_ifdef EQUAL -1)
    message(FATAL_ERROR "AC-4 / AC-STD-3 FAIL: '#ifdef _WIN32' found in Connection.h -- remove all platform conditionals and use PlatformLibrary.h backends")
endif()

# --- Check 5: No LoadLibrary (AC-1) ---
string(FIND "${CONNECTION_H_CONTENT}" "LoadLibrary" _pos_loadlib)
if(NOT _pos_loadlib EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL: 'LoadLibrary' found in Connection.h -- replace with mu::platform::Load()")
endif()

# --- Check 6: No dlopen (AC-1) ---
string(FIND "${CONNECTION_H_CONTENT}" "dlopen" _pos_dlopen)
if(NOT _pos_dlopen EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL: 'dlopen' found in Connection.h -- replace with mu::platform::Load()")
endif()

# --- Check 7: No GetProcAddress (AC-3) ---
string(FIND "${CONNECTION_H_CONTENT}" "GetProcAddress" _pos_getproc)
if(NOT _pos_getproc EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL: 'GetProcAddress' found in Connection.h -- replace with mu::platform::GetSymbol()")
endif()

# --- Check 8: No dlsym (AC-3) ---
string(FIND "${CONNECTION_H_CONTENT}" "dlsym" _pos_dlsym)
if(NOT _pos_dlsym EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL: 'dlsym' found in Connection.h -- replace with mu::platform::GetSymbol()")
endif()

# --- Check 9: Story reference (cross-reference — warning only) ---
string(FIND "${CONNECTION_H_CONTENT}" "3.1.2" _pos_story_ref)
string(FIND "${CONNECTION_H_CONTENT}" "3-1-2" _pos_story_ref2)
if(_pos_story_ref EQUAL -1 AND _pos_story_ref2 EQUAL -1)
    message(WARNING "AC-STD-11: Story reference '3.1.2' or '3-1-2' not found in Connection.h header -- recommended for full traceability")
endif()

# --- Check 10: mu::platform::Load is referenced (AC-1 positive check) ---
string(FIND "${CONNECTION_H_CONTENT}" "mu::platform::Load" _pos_platform_load)
if(_pos_platform_load EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL: 'mu::platform::Load' not found in Connection.h -- the refactoring must use PlatformLibrary API")
endif()

message(STATUS "AC-STD-11 PASS: 'VS1-NET-CONNECTION-XPLAT' is present in Connection.h header")
message(STATUS "AC-4 / AC-STD-3 PASS: No '#ifdef _WIN32' in Connection.h")
message(STATUS "AC-1 PASS: No LoadLibrary / dlopen -- mu::platform::Load() used")
message(STATUS "AC-3 PASS: No GetProcAddress / dlsym -- mu::platform::GetSymbol() used")
