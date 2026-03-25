# Story 7.5.1: AC-4 — ZzzInfomation.cpp unused variable declarations at line 237 resolved
# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS
#
# RED PHASE:  Test FAILS before story 7.5.1 is implemented.
#             ZzzInfomation.cpp:237 declares int Type, x, y, Dir; which are only assigned
#             but never read, triggering -Wunused-but-set-variable on Clang.
#
# GREEN PHASE: Test PASSES after fix.
#             The multi-variable declaration "int Type, x, y, Dir;" is removed or replaced
#             with a proper usage pattern.
#
# Validates:
#   AC-4 — unused variable group declaration "int Type, x, y, Dir;" is gone from file
#           (variables removed as dead code, or replaced with a used pattern)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of ZzzInfomation.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-4 FAIL: ZzzInfomation.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Check for the specific multi-variable declaration that causes the warning
# The declaration "int Type, x, y, Dir;" declares 4 variables that are only assigned, never read
string(REGEX MATCH "int[ \t]+Type,[ \t]*x,[ \t]*y,[ \t]*Dir;" unused_decl "${source_content}")
if(unused_decl)
    message(FATAL_ERROR
        "AC-4 FAIL: Unused variable group declaration found in ZzzInfomation.cpp: '${unused_decl}'\n"
        "Variables Type, x, y, Dir are assigned via token reads but never consumed.\n"
        "Fix options:\n"
        "  1. Remove the variables and token reads entirely (if data is never used)\n"
        "  2. Mark as [[maybe_unused]]: [[maybe_unused]] int Type, x, y, Dir;\n"
        "  3. Use (void) suppression: after assignments, add (void)Type; (void)x; etc.")
endif()

message(STATUS "AC-4 PASS: Unused variable declaration 'int Type, x, y, Dir' resolved in ZzzInfomation.cpp")
