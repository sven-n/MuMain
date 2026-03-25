# Story 7.6.1: AC-3 — .pcc-config.yaml build command specifies Homebrew LLVM
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if .pcc-config.yaml build command still uses Apple Clang
#             or contains the stale "Will fail on macOS" comment.
#
# GREEN PHASE: Test PASSES when:
#   - cpp-cmake.build in .pcc-config.yaml contains /opt/homebrew/opt/llvm/bin/clang
#   - The stale "Will fail on macOS" comment is absent
#
# Validates:
#   AC-3 — .pcc-config.yaml build command specifies Homebrew LLVM; no stale macOS comment

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PCC_CONFIG_FILE)
    message(FATAL_ERROR "PCC_CONFIG_FILE must be set to the path of .pcc-config.yaml")
endif()

if(NOT EXISTS "${PCC_CONFIG_FILE}")
    message(FATAL_ERROR "AC-3 FAIL: .pcc-config.yaml not found at '${PCC_CONFIG_FILE}'")
endif()

file(READ "${PCC_CONFIG_FILE}" config_content)

# Check that Homebrew LLVM path is present in the build command
string(FIND "${config_content}" "/opt/homebrew/opt/llvm/bin/clang" brew_pos)
if(brew_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAIL: .pcc-config.yaml build command does not reference /opt/homebrew/opt/llvm/bin/clang.\n"
        "The cpp-cmake.build command must explicitly specify Homebrew LLVM compiler paths.\n"
        "Fix: Add -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang and\n"
        "     -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ to the build command.")
endif()

# Check that the stale "Will fail on macOS" comment is absent
string(FIND "${config_content}" "Will fail on macOS" stale_pos)
if(NOT stale_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAIL: Stale 'Will fail on macOS' comment still present in .pcc-config.yaml.\n"
        "Story 7.6.1 removes this comment since the native macOS build now passes.\n"
        "Fix: Remove or update the comment to reflect current macOS build capability.")
endif()

message(STATUS "AC-3 PASS: .pcc-config.yaml build command uses Homebrew LLVM; no stale macOS comment")
