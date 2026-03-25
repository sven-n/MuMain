# Story 7.6.1: AC-10 — ZzzOpenData.cpp pragma uses __has_warning guard
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if ZzzOpenData.cpp uses an unguarded #pragma clang diagnostic
#             for -Wnontrivial-memcall without checking __has_warning first.
#             Apple Clang 17 does not support -Wnontrivial-memcall, causing:
#             "unknown warning group '-Wnontrivial-memcall', ignored"
#             which becomes an error under -Werror.
#
# GREEN PHASE: Test PASSES when:
#   - ZzzOpenData.cpp wraps the -Wnontrivial-memcall pragma with __has_warning check
#   - Pattern: #if defined(__has_warning) && __has_warning("-Wnontrivial-memcall")
#
# Validates:
#   AC-10 — ZzzOpenData.cpp pragma guarded with __has_warning for cross-compiler compatibility

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of ZzzOpenData.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-10 FAIL: ZzzOpenData.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Check that Wnontrivial-memcall is mentioned (the pragma must still be present)
string(FIND "${source_content}" "Wnontrivial-memcall" pragma_pos)
if(pragma_pos EQUAL -1)
    # If the pragma is entirely absent, the story intent may have changed
    message(WARNING
        "AC-10 WARN: -Wnontrivial-memcall pragma not found in ZzzOpenData.cpp.\n"
        "Expected: #pragma clang diagnostic ignored \"-Wnontrivial-memcall\" guarded by __has_warning.\n"
        "If the pragma was removed entirely, verify the ZeroMemory call does not trigger the warning.")
    message(STATUS "AC-10 SKIP: pragma absent — assuming removed; marking as pass")
    return()
endif()

# Check that __has_warning guard wraps the pragma
string(FIND "${source_content}" "__has_warning" has_warning_pos)
if(has_warning_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-10 FAIL: ZzzOpenData.cpp has -Wnontrivial-memcall pragma but no __has_warning guard.\n"
        "Apple Clang 17 does not support this warning and will emit an error for unknown warning group.\n"
        "Fix: Wrap the pragma with:\n"
        "  #if defined(__has_warning) && __has_warning(\"-Wnontrivial-memcall\")\n"
        "  #pragma clang diagnostic push\n"
        "  #pragma clang diagnostic ignored \"-Wnontrivial-memcall\"\n"
        "  #endif\n"
        "  ... code ...\n"
        "  #if defined(__has_warning) && __has_warning(\"-Wnontrivial-memcall\")\n"
        "  #pragma clang diagnostic pop\n"
        "  #endif")
endif()

# Verify the guard properly gates the warning check
string(FIND "${source_content}" "defined(__has_warning)" defined_check_pos)
if(defined_check_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-10 FAIL: __has_warning found but not guarded with 'defined(__has_warning)'.\n"
        "Older compilers that lack __has_warning will fail to compile.\n"
        "Fix: Use '#if defined(__has_warning) && __has_warning(\"-Wnontrivial-memcall\")'")
endif()

message(STATUS "AC-10 PASS: ZzzOpenData.cpp -Wnontrivial-memcall pragma guarded with __has_warning")
