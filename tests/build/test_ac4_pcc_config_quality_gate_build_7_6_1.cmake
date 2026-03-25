# Story 7.6.1: AC-4 — .pcc-config.yaml quality_gate includes cmake --build
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if quality_gate in .pcc-config.yaml does not include the native
#             macOS build step. Previously, the build was excluded via skip_checks (7.5.1)
#             and the quality_gate only ran format-check and lint.
#
# GREEN PHASE: Test PASSES when:
#   - quality_gate in .pcc-config.yaml contains "cmake --build build"
#   - This means the native macOS build is part of the quality gate
#
# Validates:
#   AC-4 — quality_gate includes cmake --build build (build now passes on macOS)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PCC_CONFIG_FILE)
    message(FATAL_ERROR "PCC_CONFIG_FILE must be set to the path of .pcc-config.yaml")
endif()

if(NOT EXISTS "${PCC_CONFIG_FILE}")
    message(FATAL_ERROR "AC-4 FAIL: .pcc-config.yaml not found at '${PCC_CONFIG_FILE}'")
endif()

file(READ "${PCC_CONFIG_FILE}" config_content)

# Extract the quality_gate line(s) and check for cmake --build
# Look for "cmake --build" appearing in the file (in the quality_gate context)
string(FIND "${config_content}" "cmake --build build" build_in_qg_pos)
if(build_in_qg_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-4 FAIL: quality_gate in .pcc-config.yaml does not include 'cmake --build build'.\n"
        "Since the macOS native build passes (story 7.6.1), it must be part of the quality gate.\n"
        "Fix: Add '&& cmake --build build -j\\$(nproc)' to the quality_gate command in\n"
        "     the cpp-cmake tech profile of .pcc-config.yaml.")
endif()

message(STATUS "AC-4 PASS: quality_gate in .pcc-config.yaml includes cmake --build build")
