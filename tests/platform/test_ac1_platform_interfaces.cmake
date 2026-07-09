# Story 2.1.1: AC-1, AC-2, AC-5 — Platform interface headers exist
# RED PHASE: Interface headers not yet created.
#
# Validates that:
# 1. IPlatformWindow.h exists in Platform/
# 2. IPlatformEventLoop.h exists in Platform/
# 3. MuPlatform.h exists in Platform/
# 4. All use #pragma once

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PLATFORM_DIR)
    message(FATAL_ERROR "PLATFORM_DIR must be set to the Platform source directory")
endif()

set(REQUIRED_HEADERS
    "IPlatformWindow.h"
    "IPlatformEventLoop.h"
    "MuPlatform.h"
)

foreach(header ${REQUIRED_HEADERS})
    set(header_path "${PLATFORM_DIR}/${header}")

    if(NOT EXISTS "${header_path}")
        message(FATAL_ERROR "AC-1/AC-2/AC-5 FAIL: ${header} does not exist in Platform/")
    endif()

    file(READ "${header_path}" header_content)

    # Check #pragma once
    string(FIND "${header_content}" "#pragma once" pragma_pos)
    if(pragma_pos EQUAL -1)
        message(FATAL_ERROR "AC-STD-1 FAIL: ${header} missing #pragma once")
    endif()
endforeach()

# Check MuPlatform.h has mu::MuPlatform namespace/class
file(READ "${PLATFORM_DIR}/MuPlatform.h" muplatform_content)

string(FIND "${muplatform_content}" "MuPlatform" class_pos)
if(class_pos EQUAL -1)
    message(FATAL_ERROR "AC-5 FAIL: MuPlatform.h does not contain MuPlatform class/namespace")
endif()

# Check for required static methods in MuPlatform.h
string(FIND "${muplatform_content}" "CreateWindow" create_pos)
if(create_pos EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL: MuPlatform.h missing CreateWindow declaration")
endif()

string(FIND "${muplatform_content}" "PollEvents" poll_pos)
if(poll_pos EQUAL -1)
    message(FATAL_ERROR "AC-2 FAIL: MuPlatform.h missing PollEvents declaration")
endif()

string(FIND "${muplatform_content}" "GetWindow" getwindow_pos)
if(getwindow_pos EQUAL -1)
    message(FATAL_ERROR "AC-5 FAIL: MuPlatform.h missing GetWindow declaration")
endif()

string(FIND "${muplatform_content}" "Initialize" init_pos)
if(init_pos EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL: MuPlatform.h missing Initialize declaration")
endif()

string(FIND "${muplatform_content}" "Shutdown" shutdown_pos)
if(shutdown_pos EQUAL -1)
    message(FATAL_ERROR "AC-6 FAIL: MuPlatform.h missing Shutdown declaration")
endif()

message(STATUS "AC-1/AC-2/AC-5 PASS: All platform interface headers exist with correct declarations")
