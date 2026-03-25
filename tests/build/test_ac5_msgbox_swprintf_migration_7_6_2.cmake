# Flow Code: VS0-QUAL-WIN32CLEAN-STRINCLUDE
# Story 7.6.2: Win32 String Conversion and Include Guard Cleanup
# AC-5: GameShop/MsgBoxIGSBuyConfirm.cpp uses mu_swprintf instead of StringCbPrintf/StringCchPrintf;
#        <strsafe.h> include and #ifdef _WIN32 block removed.
#
# RED PHASE: Fails until Task 4 (MsgBoxIGSBuyConfirm.cpp) is complete.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "AC-5: SOURCE_FILE variable not defined")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-5: MsgBoxIGSBuyConfirm.cpp not found at: ${SOURCE_FILE}")
endif()

file(READ "${SOURCE_FILE}" content)

# Check 1: StringCbPrintf must NOT appear
string(FIND "${content}" "StringCbPrintf" _pos_cb)
if(NOT _pos_cb EQUAL -1)
    message(FATAL_ERROR "AC-5 FAILED: MsgBoxIGSBuyConfirm.cpp still uses StringCbPrintf — replace with mu_swprintf")
endif()

# Check 2: StringCchPrintf must NOT appear
string(FIND "${content}" "StringCchPrintf" _pos_cch)
if(NOT _pos_cch EQUAL -1)
    message(FATAL_ERROR "AC-5 FAILED: MsgBoxIGSBuyConfirm.cpp still uses StringCchPrintf — replace with mu_swprintf")
endif()

# Check 3: <strsafe.h> must NOT be included
string(REGEX MATCH "#include[ \t]*[<\"]strsafe\\.h[>\"]" _match_strsafe "${content}")
if(_match_strsafe)
    message(FATAL_ERROR "AC-5 FAILED: MsgBoxIGSBuyConfirm.cpp still includes <strsafe.h> — remove this include")
endif()

# Check 4: mu_swprintf MUST appear (replacement for StringCbPrintf)
string(FIND "${content}" "mu_swprintf" _pos_mu_sw)
if(_pos_mu_sw EQUAL -1)
    message(FATAL_ERROR "AC-5 FAILED: MsgBoxIGSBuyConfirm.cpp does not use mu_swprintf — migration not complete")
endif()

message(STATUS "AC-5 PASSED: MsgBoxIGSBuyConfirm.cpp uses mu_swprintf and has no StringCbPrintf or <strsafe.h>")
