# Story 7.5.1: AC-9 — .pcc-config.yaml quality gate bypass removed
# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS
#
# RED PHASE:  Test FAILS before story 7.5.1 is implemented.
#             .pcc-config.yaml contains skip_checks: [build, test] which allows the quality gate
#             to pass without running the native macOS build or tests.
#
# GREEN PHASE: Test PASSES after fix.
#             skip_checks key is removed from .pcc-config.yaml entirely.
#
# Validates:
#   AC-9 — skip_checks no longer in .pcc-config.yaml quality_gates.backend section

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ROOT)
    message(FATAL_ERROR "ROOT must be set to the workspace root directory")
endif()

set(PCC_CONFIG_FILE "${ROOT}/.pcc-config.yaml")

if(NOT EXISTS "${PCC_CONFIG_FILE}")
    message(FATAL_ERROR "AC-9 FAIL: .pcc-config.yaml not found at '${PCC_CONFIG_FILE}'")
endif()

file(READ "${PCC_CONFIG_FILE}" config_content)

# Check that skip_checks is not present (the bypass must be removed entirely)
string(FIND "${config_content}" "skip_checks" skip_pos)
if(NOT skip_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-9 FAIL: 'skip_checks' key still present in .pcc-config.yaml.\n"
        "The quality gate bypass must be removed entirely.\n"
        "Fix: Remove the line 'skip_checks: [build, test]' from quality_gates.backend section.\n"
        "After removal, the cpp-cmake quality_gate command must include native build verification.")
endif()

message(STATUS "AC-9 PASS: skip_checks not found in .pcc-config.yaml — quality gate bypass removed")
