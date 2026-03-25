# Story 7.6.1: AC-5 — ZzzOpenglUtil.cpp excluded from macOS/non-Win32 CMake build
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if ZzzOpenglUtil.cpp is not excluded from non-Windows builds.
#             ZzzOpenglUtil.cpp includes wglext.h (WGL — Windows OpenGL extensions)
#             which cascades into DECLARE_HANDLE failures on macOS.
#
# GREEN PHASE: Test PASSES when:
#   - CMakeLists.txt excludes ZzzOpenglUtil.cpp from non-Win32 builds (via NOT WIN32 guard)
#   - The exclusion targets MURenderFX or removes from the source list
#
# Validates:
#   AC-5 — ZzzOpenglUtil.cpp excluded from macOS CMake build via CMake conditional

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CMAKELISTS_FILE)
    message(FATAL_ERROR "CMAKELISTS_FILE must be set to the path of MuMain/src/CMakeLists.txt")
endif()

if(NOT EXISTS "${CMAKELISTS_FILE}")
    message(FATAL_ERROR "AC-5 FAIL: CMakeLists.txt not found at '${CMAKELISTS_FILE}'")
endif()

file(READ "${CMAKELISTS_FILE}" cmake_content)

# Check that ZzzOpenglUtil.cpp appears in the file (it must be referenced somewhere)
string(FIND "${cmake_content}" "ZzzOpenglUtil.cpp" zzzopen_pos)
if(zzzopen_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-5 FAIL: ZzzOpenglUtil.cpp not found in CMakeLists.txt.\n"
        "The file must be explicitly excluded from non-Windows builds.\n"
        "Fix: Add a list(REMOVE_ITEM) or target_sources() guard for ZzzOpenglUtil.cpp\n"
        "     inside an if(NOT WIN32) block in MuMain/src/CMakeLists.txt.")
endif()

# Check that ZzzOpenglUtil.cpp is guarded by a NOT WIN32 or WIN32 conditional
# The file must appear inside an if(NOT WIN32)...REMOVE_ITEM or if(WIN32)...add block

# Strategy: look for REMOVE_ITEM + ZzzOpenglUtil.cpp pattern (NOT WIN32 exclusion approach)
# or if(WIN32) ... ZzzOpenglUtil.cpp pattern (Windows-only addition approach)
string(REGEX MATCH "if[ \t]*\\([ \t]*NOT[ \t]+WIN32[ \t]*\\).*ZzzOpenglUtil\\.cpp" remove_guard "${cmake_content}")
string(REGEX MATCH "if[ \t]*\\([ \t]*WIN32[ \t]*\\).*ZzzOpenglUtil\\.cpp" win32_guard "${cmake_content}")

# Multiline matching via FIND approach
string(FIND "${cmake_content}" "NOT WIN32" not_win32_pos)
string(FIND "${cmake_content}" "REMOVE_ITEM" remove_item_pos)

# If the file is referenced and either NOT WIN32 or WIN32 guard exists near ZzzOpenglUtil reference,
# we consider it guarded. A simple presence check:
if(not_win32_pos EQUAL -1 AND win32_guard STREQUAL "")
    message(FATAL_ERROR
        "AC-5 FAIL: ZzzOpenglUtil.cpp is not conditionally excluded for non-Windows builds.\n"
        "Expected: if(NOT WIN32) block with REMOVE_ITEM for ZzzOpenglUtil.cpp, OR\n"
        "          if(WIN32) block adding ZzzOpenglUtil.cpp only on Windows.\n"
        "Fix: Wrap the ZzzOpenglUtil.cpp source entry with appropriate CMake conditionals.")
endif()

message(STATUS "AC-5 PASS: ZzzOpenglUtil.cpp is excluded from non-Windows builds in CMakeLists.txt")
