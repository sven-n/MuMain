# Story 7.6.1: AC-2 — ./ctl build uses Homebrew Clang from /opt/homebrew/opt/llvm/bin/
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if ./ctl uses Apple Clang (/usr/bin/c++) instead of Homebrew LLVM.
#             Apple Clang 17 lacks -Wnontrivial-memcall, causing the pragma guard in
#             ZzzOpenData.cpp to fail to suppress the warning.
#
# GREEN PHASE: Test PASSES when:
#   - cmd_build() in ./ctl passes -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang
#   - cmd_build() in ./ctl passes -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++
#
# Validates:
#   AC-2 — ./ctl build explicitly uses Homebrew LLVM compiler paths

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CTL_SCRIPT)
    message(FATAL_ERROR "CTL_SCRIPT must be set to the path of the ./ctl script")
endif()

if(NOT EXISTS "${CTL_SCRIPT}")
    message(FATAL_ERROR "AC-2 FAIL: ./ctl not found at '${CTL_SCRIPT}'")
endif()

file(READ "${CTL_SCRIPT}" ctl_content)

# Check for Homebrew LLVM C compiler path in cmd_build()
string(FIND "${ctl_content}" "/opt/homebrew/opt/llvm/bin/clang" clang_pos)
if(clang_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-2 FAIL: ./ctl does not reference /opt/homebrew/opt/llvm/bin/clang.\n"
        "The build command must explicitly pass Homebrew LLVM compiler paths.\n"
        "Fix: In cmd_build(), add -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang\n"
        "     and -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++")
endif()

# Check for Homebrew LLVM C++ compiler path in cmd_build()
string(FIND "${ctl_content}" "/opt/homebrew/opt/llvm/bin/clang++" clangxx_pos)
if(clangxx_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-2 FAIL: ./ctl does not reference /opt/homebrew/opt/llvm/bin/clang++.\n"
        "Fix: In cmd_build(), add -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++")
endif()

message(STATUS "AC-2 PASS: ./ctl build uses Homebrew LLVM at /opt/homebrew/opt/llvm/bin/")
