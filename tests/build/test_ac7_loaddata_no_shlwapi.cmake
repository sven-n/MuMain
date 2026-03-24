# Story 7.3.0: AC-7 — LoadData.cpp has no #include <shlwapi.h>
# Flow Code: VS0-QUAL-BUILDCOMPAT-MACOS
#
# RED PHASE:  Test FAILS before story 7.3.0 is implemented.
#             LoadData.cpp includes <shlwapi.h> (Windows Shell API) but does not use it,
#             causing "No such file or directory" error on macOS/Linux.
#
# GREEN PHASE: Test PASSES after fix.
#             The #include <shlwapi.h> line is removed from LoadData.cpp.
#
# Validates:
#   AC-7 — no #include <shlwapi.h> in LoadData.cpp

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of LoadData.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-7 FAIL: LoadData.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Check for shlwapi.h include
string(FIND "${source_content}" "shlwapi.h" shlwapi_pos)
if(NOT shlwapi_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-7 FAIL: '#include <shlwapi.h>' found in LoadData.cpp at position ${shlwapi_pos}.\n"
        "shlwapi.h is a Windows-only Shell API header and is not used in LoadData.cpp.\n"
        "Fix: remove the #include <shlwapi.h> line.")
endif()

message(STATUS "AC-7 PASS: shlwapi.h include removed from LoadData.cpp")
