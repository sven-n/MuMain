# Story 1.2.1: AC-3 — Verify platform headers use #pragma once
# GREEN PHASE: All platform headers created and verified.
#
# Usage: cmake -DPLATFORM_DIR=<path-to-Platform-dir> -P test_ac3_pragma_once.cmake

if(NOT DEFINED PLATFORM_DIR)
    message(FATAL_ERROR "PLATFORM_DIR must be defined")
endif()

foreach(header PlatformTypes.h PlatformKeys.h PlatformCompat.h)
    set(header_path "${PLATFORM_DIR}/${header}")

    if(NOT EXISTS "${header_path}")
        message(FATAL_ERROR "AC-3 FAIL: ${header} does not exist at ${header_path}")
    endif()

    file(READ "${header_path}" content)

    string(FIND "${content}" "#pragma once" pos)
    if(pos LESS 0)
        message(FATAL_ERROR "AC-3 FAIL: ${header} does not contain #pragma once")
    endif()

    message(STATUS "AC-3: ${header} has #pragma once")
endforeach()

message(STATUS "AC-3: All platform headers use #pragma once")
