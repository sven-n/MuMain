# Story 7.5.1: AC-3 — ZzzInfomation.cpp wchar_t null comparisons use L'\0' not NULL
# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS
#
# RED PHASE:  Test FAILS before story 7.5.1 is implemented.
#             Lines 91, 139, 346 compare wchar_t array elements with NULL (integer 0),
#             triggering -Wnull-arithmetic / -Wpointer-integer-compare on Clang.
#
# GREEN PHASE: Test PASSES after fix.
#             All wchar_t null checks use L'\0' instead of NULL.
#
# Validates:
#   AC-3 — AbuseFilter[i][0] == NULL, AbuseNameFilter[i][0] == NULL,
#           p->Name[0] != NULL replaced with L'\0' equivalents

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of ZzzInfomation.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-3 FAIL: ZzzInfomation.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

set(found_violations FALSE)

# Check for AbuseFilter array element compared with NULL (wchar_t vs integer NULL)
string(REGEX MATCH "AbuseFilter\\[[^]]+\\]\\[0\\][ \t]*[=!]=[ \t]*NULL" filter_null "${source_content}")
if(filter_null)
    message(WARNING "AC-3 FAIL: wchar_t AbuseFilter element compared with NULL: '${filter_null}'")
    set(found_violations TRUE)
endif()

# Check for AbuseNameFilter array element compared with NULL
string(REGEX MATCH "AbuseNameFilter\\[[^]]+\\]\\[0\\][ \t]*[=!]=[ \t]*NULL" namefilter_null "${source_content}")
if(namefilter_null)
    message(WARNING "AC-3 FAIL: wchar_t AbuseNameFilter element compared with NULL: '${namefilter_null}'")
    set(found_violations TRUE)
endif()

# Check for wchar_t member Name[0] compared with NULL
string(REGEX MATCH "->Name\\[0\\][ \t]*[=!]=[ \t]*NULL" name_null "${source_content}")
if(name_null)
    message(WARNING "AC-3 FAIL: wchar_t Name[0] compared with NULL: '${name_null}'")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-3 FAIL: wchar_t null comparisons using NULL found in ZzzInfomation.cpp.\n"
        "NULL is integer 0; comparing with wchar_t triggers -Wnull-arithmetic on Clang.\n"
        "Fix: Replace == NULL with == L'\\0' and != NULL with != L'\\0' for wchar_t elements.\n"
        "Affected locations: lines 91 (AbuseFilter), 139 (AbuseNameFilter), 346 (p->Name).")
endif()

message(STATUS "AC-3 PASS: No wchar_t == NULL / != NULL comparisons found in ZzzInfomation.cpp")
