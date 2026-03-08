# AC-STD-11: Flow Code Traceability — VS1-NET-CONFIG-SERVER
# AC-VAL-4:  GameConfig.cpp verified for flow code presence
#
# Story: 3.4.2 - Server Connection Configuration
#
# Validates that:
#   1. GameConfig.cpp exists at the expected path
#   2. Contains the VS1-NET-CONFIG-SERVER flow code anywhere in the file
#   3. Contains the VS1-NET-CONFIG-SERVER flow code in the header block (first 1000 chars)
#   4. test_server_config_validation.cpp also contains the flow code (test file traceability)
#
# Run manually: cmake -P tests/build/test_ac_std11_flow_code_3_4_2.cmake

cmake_minimum_required(VERSION 3.25)

set(GAMECONFIG_CPP "${CMAKE_CURRENT_LIST_DIR}/../../src/source/Data/GameConfig.cpp")
set(TEST_FILE "${CMAKE_CURRENT_LIST_DIR}/../../tests/network/test_server_config_validation.cpp")

# --- Check 1: GameConfig.cpp exists ---
if(NOT EXISTS "${GAMECONFIG_CPP}")
    message(FATAL_ERROR "AC-STD-11 FAIL: GameConfig.cpp not found at '${GAMECONFIG_CPP}' -- file must exist before ATDD checks can pass")
endif()

file(READ "${GAMECONFIG_CPP}" GAMECONFIG_CONTENT)

# --- Check 2: VS1-NET-CONFIG-SERVER flow code is present in GameConfig.cpp ---
if(NOT GAMECONFIG_CONTENT MATCHES "VS1-NET-CONFIG-SERVER")
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-CONFIG-SERVER' not found in GameConfig.cpp -- add '// Flow Code: VS1-NET-CONFIG-SERVER' to the file header comment")
endif()

# --- Check 3: Flow code appears in GameConfig.cpp header area (first 1000 chars) ---
string(SUBSTRING "${GAMECONFIG_CONTENT}" 0 1000 _gameconfig_header)
if(NOT _gameconfig_header MATCHES "VS1-NET-CONFIG-SERVER")
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-CONFIG-SERVER' not found in the first 1000 chars of GameConfig.cpp -- flow code must appear in the file header comment block")
endif()

# --- Check 4: test_server_config_validation.cpp exists ---
if(NOT EXISTS "${TEST_FILE}")
    message(FATAL_ERROR "AC-STD-11 FAIL: test_server_config_validation.cpp not found at '${TEST_FILE}' -- test file must be created")
endif()

file(READ "${TEST_FILE}" TEST_CONTENT)

# --- Check 5: Flow code present in test file ---
if(NOT TEST_CONTENT MATCHES "VS1-NET-CONFIG-SERVER")
    message(FATAL_ERROR "AC-STD-11 FAIL: 'VS1-NET-CONFIG-SERVER' not found in test_server_config_validation.cpp -- add flow code to test file header")
endif()

message(STATUS "AC-STD-11 PASS: VS1-NET-CONFIG-SERVER flow code present in GameConfig.cpp header")
message(STATUS "AC-VAL-4 PASS: GameConfig.cpp exists and contains required flow code traceability")
message(STATUS "AC-STD-11 PASS: VS1-NET-CONFIG-SERVER flow code present in test_server_config_validation.cpp")
message(STATUS "")
message(STATUS "=== 3.4.2 AC-STD-11 / AC-VAL-4 PASS ===")
message(STATUS "  GameConfig.cpp: VS1-NET-CONFIG-SERVER flow code present in header")
message(STATUS "  test_server_config_validation.cpp: VS1-NET-CONFIG-SERVER flow code present")
message(STATUS "=========================================")
