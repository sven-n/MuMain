# AC-STD-11: Flow Code Traceability — VS1-NET-ERROR-MESSAGING
# AC-VAL-4:  Connection.cpp verified for flow code presence
#
# Story: 3.4.1 - Connection Error Messaging & Graceful Degradation
#
# Validates that:
#   1. Connection.cpp exists at the expected path
#   2. Contains the VS1-NET-ERROR-MESSAGING flow code anywhere in the file
#   3. Contains the VS1-NET-ERROR-MESSAGING flow code in the header block (first 1000 chars)
#   4. test_connection_error_messages.cpp also contains the flow code (test file traceability)
#
# Run manually: cmake -P tests/build/test_ac_std11_flow_code_3_4_1.cmake

cmake_minimum_required(VERSION 3.25)

set(CONNECTION_CPP "${CMAKE_CURRENT_LIST_DIR}/../../src/source/Dotnet/Connection.cpp")
set(TEST_FILE "${CMAKE_CURRENT_LIST_DIR}/../../tests/network/test_connection_error_messages.cpp")

# --- Check 1: Connection.cpp exists ---
if(NOT EXISTS "${CONNECTION_CPP}")
    message(FATAL_ERROR "AC-STD-11 FAIL: Connection.cpp not found at '${CONNECTION_CPP}' -- file must exist before ATDD checks can pass")
endif()

file(READ "${CONNECTION_CPP}" CONNECTION_CONTENT)

# --- Check 2: VS1-NET-ERROR-MESSAGING flow code is present in Connection.cpp ---
if(NOT CONNECTION_CONTENT MATCHES "VS1-NET-ERROR-MESSAGING")
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-ERROR-MESSAGING' not found in Connection.cpp -- add '// Flow Code: VS1-NET-ERROR-MESSAGING' to the file header comment")
endif()

# --- Check 3: Flow code appears in Connection.cpp header area (first 1000 chars) ---
string(SUBSTRING "${CONNECTION_CONTENT}" 0 1000 _connection_header)
if(NOT _connection_header MATCHES "VS1-NET-ERROR-MESSAGING")
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-ERROR-MESSAGING' not found in the first 1000 chars of Connection.cpp -- flow code must appear in the file header comment block")
endif()

# --- Check 4: test_connection_error_messages.cpp exists ---
if(NOT EXISTS "${TEST_FILE}")
    message(FATAL_ERROR "AC-STD-11 FAIL: test_connection_error_messages.cpp not found at '${TEST_FILE}' -- test file must be created")
endif()

file(READ "${TEST_FILE}" TEST_CONTENT)

# --- Check 5: Flow code present in test file ---
if(NOT TEST_CONTENT MATCHES "VS1-NET-ERROR-MESSAGING")
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-ERROR-MESSAGING' not found in test_connection_error_messages.cpp -- add flow code to test file header")
endif()

message(STATUS "AC-STD-11 PASS: VS1-NET-ERROR-MESSAGING flow code present in Connection.cpp header")
message(STATUS "AC-VAL-4 PASS: Connection.cpp exists and contains required flow code traceability")
message(STATUS "AC-STD-11 PASS: VS1-NET-ERROR-MESSAGING flow code present in test_connection_error_messages.cpp")
message(STATUS "")
message(STATUS "=== 3.4.1 AC-STD-11 / AC-VAL-4 PASS ===")
message(STATUS "  Connection.cpp: VS1-NET-ERROR-MESSAGING flow code present in header")
message(STATUS "  test_connection_error_messages.cpp: VS1-NET-ERROR-MESSAGING flow code present")
message(STATUS "=========================================")
