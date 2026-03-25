# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-3: .pcc-config.yaml build command specifies Homebrew LLVM

cmake_minimum_required(VERSION 3.23)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set(PCC_CONFIG "${WORKSPACE_ROOT}/.pcc-config.yaml")

if(NOT EXISTS "${PCC_CONFIG}")
  message(FATAL_ERROR "PCC config not found")
endif()

file(READ "${PCC_CONFIG}" pcc_content)

if(NOT pcc_content MATCHES "build.*homebrew.*llvm")
  message(FATAL_ERROR "AC-3 FAILED: .pcc-config.yaml build command does not reference Homebrew LLVM")
endif()

if(pcc_content MATCHES "Will fail on macOS.*Win32 APIs")
  message(FATAL_ERROR "AC-3 FAILED: Stale comment still present")
endif()

message(STATUS "AC-3 PASSED")
