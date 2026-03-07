# AC-STD-11: Flow Code Traceability — VS1-NET-CHAR16T-ENCODING
# AC-1 / AC-3 / AC-STD-3: No wchar_t at .NET interop boundary
# AC-VAL-3: Connection.h and Common.xslt validated for char16_t encoding
#
# Story: 3.2.1 - char16_t Encoding at .NET Interop Boundary
#
# RED PHASE: This test FAILS until the following are implemented:
#   - Connection.h: constructor uses const char16_t* host, flow code comment present
#   - Connection.h: Connect typedef uses const char16_t*
#   - Common.xslt: String nativetype produces const char16_t* (not const wchar_t*)
#
# Run manually: cmake -P tests/build/test_ac_std11_flow_code_3_2_1.cmake

cmake_minimum_required(VERSION 3.25)

# Paths passed from CTest or defaulted relative to this script
if(NOT DEFINED CONNECTION_H)
    set(CONNECTION_H "${CMAKE_CURRENT_LIST_DIR}/../../src/source/Dotnet/Connection.h")
endif()

if(NOT DEFINED COMMON_XSLT)
    set(COMMON_XSLT "${CMAKE_CURRENT_LIST_DIR}/../../ClientLibrary/Common.xslt")
endif()

# ==========================================================================
# CONNECTION_H CHECKS
# ==========================================================================

# --- Check 1: Connection.h exists ---
if(NOT EXISTS "${CONNECTION_H}")
    message(FATAL_ERROR "AC-STD-11 FAIL: Connection.h not found at '${CONNECTION_H}' -- file must exist before ATDD checks can run")
endif()

file(READ "${CONNECTION_H}" CONNECTION_H_CONTENT)

# --- Check 2: Flow code VS1-NET-CHAR16T-ENCODING is present (AC-STD-11) ---
string(FIND "${CONNECTION_H_CONTENT}" "VS1-NET-CHAR16T-ENCODING" _pos_flow_code)
if(_pos_flow_code EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-CHAR16T-ENCODING' not found in Connection.h -- add '// Flow Code: VS1-NET-CHAR16T-ENCODING' comment to Connection.h (alongside VS1-NET-CONNECTION-XPLAT)")
endif()

# --- Check 3: Flow code appears in header area (first 3000 chars) ---
string(SUBSTRING "${CONNECTION_H_CONTENT}" 0 3000 _header_block)
string(FIND "${_header_block}" "VS1-NET-CHAR16T-ENCODING" _pos_in_header)
if(_pos_in_header EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-CHAR16T-ENCODING' not found in the first ~3000 chars of Connection.h -- it must appear in the file header comment block, not buried in code")
endif()

# --- Check 4: const char16_t* appears in Connection.h (AC-1 / AC-STD-3) ---
string(FIND "${CONNECTION_H_CONTENT}" "const char16_t*" _pos_char16)
if(_pos_char16 EQUAL -1)
    message(FATAL_ERROR "AC-1 / AC-STD-3 FAIL: 'const char16_t*' not found in Connection.h -- constructor declaration and Connect typedef must use char16_t")
endif()

# --- Check 5: const wchar_t* does NOT appear in Connection.h (AC-1 / AC-STD-3) ---
# Exception: wchar_t may appear in comments explaining the migration; check for
# the specific interop-boundary pattern only.
string(FIND "${CONNECTION_H_CONTENT}" "const wchar_t*" _pos_wchar_param)
if(NOT _pos_wchar_param EQUAL -1)
    message(FATAL_ERROR "AC-1 / AC-STD-3 FAIL: 'const wchar_t*' found in Connection.h -- the constructor parameter and Connect typedef must use 'const char16_t*', not 'const wchar_t*'. Remove all wchar_t* at the interop boundary.")
endif()

message(STATUS "AC-STD-11 PASS: 'VS1-NET-CHAR16T-ENCODING' is present in Connection.h header")
message(STATUS "AC-1 PASS: 'const char16_t*' found in Connection.h (constructor + typedef)")
message(STATUS "AC-STD-3 PASS: 'const wchar_t*' not present in Connection.h interop boundary")

# ==========================================================================
# COMMON_XSLT CHECKS
# ==========================================================================

# --- Check 6: Common.xslt exists ---
if(NOT EXISTS "${COMMON_XSLT}")
    message(FATAL_ERROR "AC-3 FAIL: Common.xslt not found at '${COMMON_XSLT}' -- file must exist")
endif()

file(READ "${COMMON_XSLT}" COMMON_XSLT_CONTENT)

# --- Check 7: const char16_t* appears in Common.xslt String nativetype (AC-3) ---
string(FIND "${COMMON_XSLT_CONTENT}" "const char16_t*" _pos_xslt_char16)
if(_pos_xslt_char16 EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL: 'const char16_t*' not found in Common.xslt -- the String nativetype template must produce 'const char16_t*' instead of 'const wchar_t*'")
endif()

# --- Check 8: const wchar_t* does NOT appear in Common.xslt nativetype (AC-3 / AC-STD-3) ---
# We check specifically for the nativetype template for String.
# A comment mentioning wchar_t would be acceptable, but 'const wchar_t*' as a
# template output is not. Use a targeted search for the nativetype value pattern.
string(FIND "${COMMON_XSLT_CONTENT}" ">const wchar_t*<" _pos_xslt_wchar_template)
if(NOT _pos_xslt_wchar_template EQUAL -1)
    message(FATAL_ERROR "AC-3 / AC-STD-3 FAIL: '>const wchar_t*<' found in Common.xslt -- the String nativetype template must output 'const char16_t*', not 'const wchar_t*'. Update line 94 of Common.xslt.")
endif()

message(STATUS "AC-3 PASS: 'const char16_t*' found in Common.xslt String nativetype template")
message(STATUS "AC-STD-3 PASS: '>const wchar_t*<' not found as nativetype output in Common.xslt")

# ==========================================================================
# SUMMARY
# ==========================================================================

message(STATUS "")
message(STATUS "=== 3.2.1 AC-STD-11 / AC-VAL-3 PASS ===")
message(STATUS "  Connection.h: VS1-NET-CHAR16T-ENCODING flow code present")
message(STATUS "  Connection.h: const char16_t* at interop boundary (no const wchar_t*)")
message(STATUS "  Common.xslt: String nativetype = const char16_t* (no const wchar_t*)")
message(STATUS "=========================================")
