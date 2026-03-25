# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-2: ./ctl build uses Homebrew Clang from /opt/homebrew/opt/llvm/bin/

cmake_minimum_required(VERSION 3.23)

# Get workspace root (3 levels up from this file's directory)
get_filename_component(TEST_DIR "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
get_filename_component(WORKSPACE_ROOT "${TEST_DIR}/../../../" ABSOLUTE)

set(CTL_SCRIPT "${WORKSPACE_ROOT}/ctl")

if(NOT EXISTS "${CTL_SCRIPT}")
  message(FATAL_ERROR "ctl script not found at ${CTL_SCRIPT}")
endif()

file(READ "${CTL_SCRIPT}" ctl_content)

if(NOT ctl_content MATCHES "/opt/homebrew/opt/llvm")
  message(FATAL_ERROR "AC-2 FAILED: ctl script does not reference /opt/homebrew/opt/llvm")
endif()

message(STATUS "AC-2 PASSED: ctl script correctly references Homebrew LLVM paths")
