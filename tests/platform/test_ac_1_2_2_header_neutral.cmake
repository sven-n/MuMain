# Story 1.2.2 AC-STD-3: Verify PlatformLibrary.h has no platform conditionals
# RED PHASE: Will fail until PlatformLibrary.h is created.

if(NOT DEFINED PLATFORM_DIR)
    message(FATAL_ERROR "PLATFORM_DIR not set")
endif()

set(HEADER_FILE "${PLATFORM_DIR}/PlatformLibrary.h")

# Check file exists
if(NOT EXISTS "${HEADER_FILE}")
    message(FATAL_ERROR "AC-STD-3 FAIL: PlatformLibrary.h does not exist at ${HEADER_FILE}")
endif()

# Read the header content
file(READ "${HEADER_FILE}" HEADER_CONTENT)

# Verify no #ifdef _WIN32 in the header
string(FIND "${HEADER_CONTENT}" "#ifdef _WIN32" WIN32_POS)
if(NOT WIN32_POS EQUAL -1)
    message(FATAL_ERROR "AC-STD-3 FAIL: PlatformLibrary.h contains #ifdef _WIN32 — platform conditionals must only be in backend .cpp files")
endif()

# Verify no #ifdef __linux__ in the header
string(FIND "${HEADER_CONTENT}" "#ifdef __linux__" LINUX_POS)
if(NOT LINUX_POS EQUAL -1)
    message(FATAL_ERROR "AC-STD-3 FAIL: PlatformLibrary.h contains #ifdef __linux__")
endif()

# Verify no #ifdef __APPLE__ in the header
string(FIND "${HEADER_CONTENT}" "#ifdef __APPLE__" APPLE_POS)
if(NOT APPLE_POS EQUAL -1)
    message(FATAL_ERROR "AC-STD-3 FAIL: PlatformLibrary.h contains #ifdef __APPLE__")
endif()

# Verify no windows.h include
string(FIND "${HEADER_CONTENT}" "windows.h" WINDOWS_H_POS)
if(NOT WINDOWS_H_POS EQUAL -1)
    message(FATAL_ERROR "AC-STD-3 FAIL: PlatformLibrary.h includes windows.h")
endif()

# Verify no dlfcn.h include
string(FIND "${HEADER_CONTENT}" "dlfcn.h" DLFCN_H_POS)
if(NOT DLFCN_H_POS EQUAL -1)
    message(FATAL_ERROR "AC-STD-3 FAIL: PlatformLibrary.h includes dlfcn.h")
endif()

# Verify #pragma once
string(FIND "${HEADER_CONTENT}" "#pragma once" PRAGMA_POS)
if(PRAGMA_POS EQUAL -1)
    message(FATAL_ERROR "AC-STD-1 FAIL: PlatformLibrary.h missing #pragma once")
endif()

# Verify mu::platform namespace
string(FIND "${HEADER_CONTENT}" "namespace mu::platform" NS_POS)
if(NS_POS EQUAL -1)
    message(FATAL_ERROR "AC-STD-1 FAIL: PlatformLibrary.h missing namespace mu::platform")
endif()

# Verify [[nodiscard]] on Load
string(FIND "${HEADER_CONTENT}" "[[nodiscard]]" NODISCARD_POS)
if(NODISCARD_POS EQUAL -1)
    message(FATAL_ERROR "AC-6 FAIL: PlatformLibrary.h missing [[nodiscard]] on fallible functions")
endif()

message(STATUS "AC-STD-3 PASS: PlatformLibrary.h is platform-neutral")
message(STATUS "AC-STD-1 PASS: PlatformLibrary.h follows naming and pragma conventions")
message(STATUS "AC-6 PASS: PlatformLibrary.h uses [[nodiscard]] on fallible functions")
