# Story 7.5.1: AC-1 — SkillDataLoader.cpp swprintf uses 3-argument POSIX form
# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS
#
# RED PHASE:  Test FAILS before story 7.5.1 is implemented.
#             SkillDataLoader.cpp calls swprintf(buf, L"fmt", ...) using the Windows 2-arg form,
#             which fails to compile on POSIX systems (macOS/Linux) requiring the 3-arg form.
#
# GREEN PHASE: Test PASSES after fix.
#             mu_swprintf(buf, L"fmt", ...) or swprintf(buf, N, L"fmt", ...) used instead.
#
# Validates:
#   AC-1 — swprintf calls in SkillDataLoader.cpp use 3-arg POSIX form or mu_swprintf

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of SkillDataLoader.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-1 FAIL: SkillDataLoader.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Check for 2-arg swprintf: pattern where second arg is a format string (starts with L")
# The problematic pattern: swprintf(buf, L"...") — format string is 2nd arg, no size param
string(REGEX MATCH "swprintf\\([^,]+,[ \t]*L\"" bad_call "${source_content}")
if(bad_call)
    message(FATAL_ERROR
        "AC-1 FAIL: swprintf uses 2-argument Windows form in SkillDataLoader.cpp.\n"
        "The POSIX 3-argument form swprintf(buf, size, fmt, ...) or mu_swprintf is required.\n"
        "Fix: Replace swprintf(buf, L\"...\") with mu_swprintf(buf, L\"...\") or\n"
        "     swprintf(buf, _countof(buf), L\"...\") for POSIX compatibility.")
endif()

message(STATUS "AC-1 PASS: No 2-arg swprintf found in SkillDataLoader.cpp — POSIX-compatible form used")
