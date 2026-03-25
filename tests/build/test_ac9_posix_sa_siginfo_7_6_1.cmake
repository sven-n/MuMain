# Story 7.6.1: AC-9 — PosixSignalHandlers.cpp uses SA_SIGINFO (not SA_SIGACTION)
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if PosixSignalHandlers.cpp uses SA_SIGACTION.
#             SA_SIGACTION is a Linux-specific extension (GNU libc), not defined on macOS.
#             Compiling with SA_SIGACTION on macOS results in "undeclared identifier" errors.
#
# GREEN PHASE: Test PASSES when:
#   - SA_SIGACTION is NOT present as a flag constant (not just in comments)
#   - SA_SIGINFO IS present (correct POSIX flag for 3-arg signal handler form)
#
# Validates:
#   AC-9 — PosixSignalHandlers.cpp uses SA_SIGINFO throughout; SA_SIGACTION removed

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of PosixSignalHandlers.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-9 FAIL: PosixSignalHandlers.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Check that SA_SIGINFO is present (the correct flag)
string(FIND "${source_content}" "SA_SIGINFO" sa_siginfo_pos)
if(sa_siginfo_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-9 FAIL: SA_SIGINFO not found in PosixSignalHandlers.cpp.\n"
        "SA_SIGINFO is the correct POSIX flag for the 3-argument signal handler form.\n"
        "Fix: Replace SA_SIGACTION with SA_SIGINFO in all flag assignments and checks.")
endif()

# Check that SA_SIGACTION is NOT present outside of comments
# Strip single-line comments (// ...) to avoid false positives from explanatory comments
string(REGEX REPLACE "//[^\n]*" "" source_no_comments "${source_content}")
# Strip block comments (/* ... */)  — simplified approach for single-file check
string(REGEX REPLACE "/\\*[^*]*\\*/" "" source_no_comments "${source_no_comments}")

string(FIND "${source_no_comments}" "SA_SIGACTION" sa_sigaction_pos)
if(NOT sa_sigaction_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-9 FAIL: SA_SIGACTION still present in PosixSignalHandlers.cpp code (not in a comment).\n"
        "SA_SIGACTION is a Linux-specific extension not available on macOS.\n"
        "Fix: Replace all SA_SIGACTION flag usages with SA_SIGINFO.\n"
        "Note: SA_SIGACTION may remain in comments for documentation purposes only.")
endif()

message(STATUS "AC-9 PASS: PosixSignalHandlers.cpp uses SA_SIGINFO; SA_SIGACTION removed from code")
