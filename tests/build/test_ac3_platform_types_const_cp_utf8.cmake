# Story 7.3.0: AC-3 — PlatformTypes.h defines CONST and CP_UTF8 in non-Win32 #else block
# Flow Code: VS0-QUAL-BUILDCOMPAT-MACOS
#
# RED PHASE:  Test FAILS before story 7.3.0 is implemented.
#             PlatformTypes.h #else block lacks CONST and CP_UTF8 definitions,
#             causing "undeclared identifier" errors in KeyGenerater.h and encoding files.
#
# GREEN PHASE: Test PASSES after fix.
#             #define CONST const  — present in non-Win32 #else section
#             #define CP_UTF8 65001 — present in non-Win32 #else section
#
# Validates:
#   AC-3 — CONST and CP_UTF8 defined in non-Win32 path of PlatformTypes.h

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PLATFORM_TYPES_H)
    message(FATAL_ERROR "PLATFORM_TYPES_H must be set to the path of PlatformTypes.h")
endif()

if(NOT EXISTS "${PLATFORM_TYPES_H}")
    message(FATAL_ERROR "AC-3 FAIL: PlatformTypes.h not found at '${PLATFORM_TYPES_H}'")
endif()

file(READ "${PLATFORM_TYPES_H}" header_content)

# Extract the non-Win32 #else section (after the first #else, before #endif)
string(FIND "${header_content}" "#else" else_pos)
if(else_pos EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL: No #else block found in PlatformTypes.h — non-Win32 section missing")
endif()
string(LENGTH "${header_content}" content_len)
string(SUBSTRING "${header_content}" ${else_pos} ${content_len} non_win32_section)

# Check for CONST definition
string(FIND "${non_win32_section}" "define CONST" const_pos)
if(const_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAIL: '#define CONST' not found in non-Win32 #else section of PlatformTypes.h.\n"
        "Fix: add '#define CONST const' to the #else section.")
endif()

# Verify CONST maps to 'const'
string(FIND "${non_win32_section}" "define CONST const" const_value_pos)
if(const_value_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAIL: '#define CONST const' value incorrect in PlatformTypes.h.\n"
        "CONST must be defined as 'const' (lowercase).")
endif()

# Check for CP_UTF8 definition
string(FIND "${non_win32_section}" "define CP_UTF8" cp_utf8_pos)
if(cp_utf8_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAIL: '#define CP_UTF8' not found in non-Win32 #else section of PlatformTypes.h.\n"
        "Fix: add '#define CP_UTF8 65001' to the #else section.")
endif()

# Verify CP_UTF8 maps to 65001
string(FIND "${non_win32_section}" "define CP_UTF8 65001" cp_utf8_value_pos)
if(cp_utf8_value_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAIL: '#define CP_UTF8 65001' value incorrect in PlatformTypes.h.\n"
        "CP_UTF8 must be defined as 65001 (Windows UTF-8 codepage value).")
endif()

message(STATUS "AC-3 PASS: CONST and CP_UTF8 correctly defined in PlatformTypes.h non-Win32 section")
