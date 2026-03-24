# Story 7.3.0: AC-4 — PlatformCompat.h provides _wcsicmp/wcsicmp, _TRUNCATE, and OutputDebugString stubs
# Flow Code: VS0-QUAL-BUILDCOMPAT-MACOS
#
# RED PHASE:  Test FAILS before story 7.3.0 is implemented.
#             PlatformCompat.h non-Win32 section lacks _wcsicmp, _TRUNCATE, and OutputDebugString,
#             causing "undeclared identifier" errors in GlobalBitmap.cpp and GameConfig.cpp.
#
# GREEN PHASE: Test PASSES after fix.
#             _wcsicmp mapped to wcscasecmp (POSIX)
#             wcsicmp mapped to wcscasecmp (POSIX alias)
#             _TRUNCATE defined as ((size_t)-1)
#             OutputDebugString defined as inline no-op
#
# Validates:
#   AC-4 — _wcsicmp/wcsicmp, _TRUNCATE, OutputDebugString present in PlatformCompat.h non-Win32 path

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PLATFORM_COMPAT_H)
    message(FATAL_ERROR "PLATFORM_COMPAT_H must be set to the path of PlatformCompat.h")
endif()

if(NOT EXISTS "${PLATFORM_COMPAT_H}")
    message(FATAL_ERROR "AC-4 FAIL: PlatformCompat.h not found at '${PLATFORM_COMPAT_H}'")
endif()

file(READ "${PLATFORM_COMPAT_H}" header_content)

# Extract the non-Win32 #else section
string(FIND "${header_content}" "#else" else_pos)
if(else_pos EQUAL -1)
    message(FATAL_ERROR "AC-4 FAIL: No #else block found in PlatformCompat.h — non-Win32 section missing")
endif()
string(LENGTH "${header_content}" content_len)
string(SUBSTRING "${header_content}" ${else_pos} ${content_len} non_win32_section)

set(found_violations FALSE)

# Check for _wcsicmp
string(FIND "${non_win32_section}" "_wcsicmp" wcsicmp_pos)
if(wcsicmp_pos EQUAL -1)
    message(WARNING "AC-4 FAIL: '_wcsicmp' not found in PlatformCompat.h non-Win32 section")
    set(found_violations TRUE)
endif()

# Check for wcsicmp (alias without underscore)
string(FIND "${non_win32_section}" "wcsicmp" wcsicmp_bare_pos)
if(wcsicmp_bare_pos EQUAL -1)
    message(WARNING "AC-4 FAIL: 'wcsicmp' not found in PlatformCompat.h non-Win32 section")
    set(found_violations TRUE)
endif()

# Check for _TRUNCATE
string(FIND "${non_win32_section}" "_TRUNCATE" truncate_pos)
if(truncate_pos EQUAL -1)
    message(WARNING "AC-4 FAIL: '_TRUNCATE' not found in PlatformCompat.h non-Win32 section")
    set(found_violations TRUE)
endif()

# Check for OutputDebugString
string(FIND "${non_win32_section}" "OutputDebugString" ods_pos)
if(ods_pos EQUAL -1)
    message(WARNING "AC-4 FAIL: 'OutputDebugString' not found in PlatformCompat.h non-Win32 section")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-4 FAIL: Missing Win32 compat stubs in PlatformCompat.h non-Win32 section.\n"
        "Required additions:\n"
        "  #define _wcsicmp wcscasecmp\n"
        "  #define wcsicmp wcscasecmp\n"
        "  #define _TRUNCATE ((size_t)-1)\n"
        "  inline void OutputDebugString(const wchar_t* /*msg*/) {}")
endif()

message(STATUS "AC-4 PASS: _wcsicmp, wcsicmp, _TRUNCATE, OutputDebugString all present in PlatformCompat.h non-Win32 section")
