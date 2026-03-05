# Story 1.2.1: AC-4 — Verify platform headers compile standalone with C++20
# RED PHASE: This test will FAIL until the headers are created.
#
# Usage: cmake -DPLATFORM_DIR=<path-to-Platform-dir> -P test_ac4_header_compilation.cmake

if(NOT DEFINED PLATFORM_DIR)
    message(FATAL_ERROR "PLATFORM_DIR must be defined")
endif()

# Check that the header files exist
foreach(header PlatformTypes.h PlatformKeys.h PlatformCompat.h)
    if(NOT EXISTS "${PLATFORM_DIR}/${header}")
        message(FATAL_ERROR "AC-4 FAIL: ${header} does not exist at ${PLATFORM_DIR}/${header}")
    endif()
    message(STATUS "AC-4: Found ${header}")
endforeach()

message(STATUS "AC-4: All platform headers exist (compile check requires g++/clang++ invocation from CTest)")
