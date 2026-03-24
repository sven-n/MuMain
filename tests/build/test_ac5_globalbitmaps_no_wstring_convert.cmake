# Story 7.3.0: AC-5 — GlobalBitmap.cpp NarrowPath() uses mu_wchar_to_utf8 on non-Windows
# Flow Code: VS0-QUAL-BUILDCOMPAT-MACOS
#
# RED PHASE:  Test FAILS before story 7.3.0 is implemented.
#             NarrowPath() uses std::wstring_convert / std::codecvt_utf8_utf16, which are
#             deprecated in C++17 and may generate errors on clang/macOS.
#
# GREEN PHASE: Test PASSES after fix.
#             std::wstring_convert and std::codecvt_utf8 are absent from non-Win32 path.
#             mu_wchar_to_utf8 is used instead.
#
# Validates:
#   AC-5 — no wstring_convert/codecvt_utf8_utf16 in GlobalBitmap.cpp cross-platform path

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of GlobalBitmap.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-5 FAIL: GlobalBitmap.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Extract the non-Win32 cross-platform section (before the first #ifdef _WIN32)
string(FIND "${source_content}" "#ifdef _WIN32" guard_pos)
if(guard_pos EQUAL -1)
    # No guard found — entire file is tested
    set(cross_platform_section "${source_content}")
else()
    string(SUBSTRING "${source_content}" 0 ${guard_pos} cross_platform_section)
endif()

set(found_violations FALSE)

# Check for deprecated wstring_convert
string(FIND "${cross_platform_section}" "wstring_convert" wconv_pos)
if(NOT wconv_pos EQUAL -1)
    message(WARNING "AC-5 FAIL: 'wstring_convert' found in GlobalBitmap.cpp cross-platform section (position ${wconv_pos})")
    set(found_violations TRUE)
endif()

# Check for deprecated codecvt_utf8
string(FIND "${cross_platform_section}" "codecvt_utf8" codecvt_pos)
if(NOT codecvt_pos EQUAL -1)
    message(WARNING "AC-5 FAIL: 'codecvt_utf8' found in GlobalBitmap.cpp cross-platform section (position ${codecvt_pos})")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-5 FAIL: Deprecated C++ conversion APIs found in GlobalBitmap.cpp cross-platform path.\n"
        "Fix NarrowPath(): use '#ifndef _WIN32' guard, call mu_wchar_to_utf8() on non-Windows,\n"
        "keep WideCharToMultiByte() inside '#ifdef _WIN32' block only.")
endif()

# Verify mu_wchar_to_utf8 is used (expected in the NarrowPath function)
string(FIND "${source_content}" "mu_wchar_to_utf8" narrow_pos)
if(narrow_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-5 FAIL: 'mu_wchar_to_utf8' not found in GlobalBitmap.cpp.\n"
        "NarrowPath() must call mu_wchar_to_utf8() on non-Windows platforms.\n"
        "Import from PlatformCompat.h which provides this shim.")
endif()

message(STATUS "AC-5 PASS: NarrowPath() uses mu_wchar_to_utf8 — no deprecated wstring_convert/codecvt_utf8 in cross-platform path")
