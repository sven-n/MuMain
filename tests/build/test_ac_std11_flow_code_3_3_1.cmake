# AC-STD-11: Flow Code Traceability — VS1-NET-VALIDATE-MACOS
# AC-VAL-3: test_macos_connectivity.cpp verified for flow code presence
#
# Story: 3.3.1 - macOS Server Connectivity Validation
#
# Validates that test_macos_connectivity.cpp:
#   1. Exists at the expected path
#   2. Contains the VS1-NET-VALIDATE-MACOS flow code in the file header
#
# Run manually: cmake -P tests/build/test_ac_std11_flow_code_3_3_1.cmake

cmake_minimum_required(VERSION 3.25)

set(TEST_FILE "${CMAKE_CURRENT_LIST_DIR}/../../tests/platform/test_macos_connectivity.cpp")

# --- Check 1: test_macos_connectivity.cpp exists ---
if(NOT EXISTS "${TEST_FILE}")
    message(FATAL_ERROR "AC-STD-11 FAIL: test_macos_connectivity.cpp not found at '${TEST_FILE}' -- file must be created before ATDD checks can pass")
endif()

file(READ "${TEST_FILE}" CONTENT)

# --- Check 2: VS1-NET-VALIDATE-MACOS flow code is present ---
if(NOT CONTENT MATCHES "VS1-NET-VALIDATE-MACOS")
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-VALIDATE-MACOS' not found in test_macos_connectivity.cpp -- add '// Flow Code: VS1-NET-VALIDATE-MACOS' to the file header comment")
endif()

# --- Check 3: Flow code appears in header area (first 1000 chars) ---
string(SUBSTRING "${CONTENT}" 0 1000 _header_block)
if(NOT _header_block MATCHES "VS1-NET-VALIDATE-MACOS")
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-VALIDATE-MACOS' not found in the first 1000 chars of test_macos_connectivity.cpp -- flow code must appear in the file header comment block")
endif()

message(STATUS "AC-STD-11 PASS: VS1-NET-VALIDATE-MACOS flow code present in test_macos_connectivity.cpp")
message(STATUS "AC-VAL-3 PASS: test_macos_connectivity.cpp exists and contains required flow code traceability")
message(STATUS "")
message(STATUS "=== 3.3.1 AC-STD-11 / AC-VAL-3 PASS ===")
message(STATUS "  test_macos_connectivity.cpp: VS1-NET-VALIDATE-MACOS flow code present")
message(STATUS "=========================================")
