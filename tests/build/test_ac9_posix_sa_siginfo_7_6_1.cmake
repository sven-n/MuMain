# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-9: PosixSignalHandlers.cpp uses SA_SIGINFO

cmake_minimum_required(VERSION 3.23)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set(POSIX_HANDLERS "${WORKSPACE_ROOT}/MuMain/src/source/Platform/posix/PosixSignalHandlers.cpp")

if(NOT EXISTS "${POSIX_HANDLERS}")
  message(FATAL_ERROR "PosixSignalHandlers.cpp not found")
endif()

file(READ "${POSIX_HANDLERS}" posix_content)

if(NOT posix_content MATCHES "SA_SIGINFO")
  message(FATAL_ERROR "AC-9 FAILED: SA_SIGINFO not found")
endif()

if(posix_content MATCHES "SA_SIGACTION")
  message(FATAL_ERROR "AC-9 FAILED: SA_SIGACTION still present")
endif()

message(STATUS "AC-9 PASSED")
