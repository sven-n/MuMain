# AC-STD-11: Flow Code Traceability — VS1-NET-CMAKE-RID
#
# Story: 3.1.1 - CMake RID Detection & .NET AOT Build Integration
# RED phase: This test FAILS until cmake/FindDotnetAOT.cmake is created
#            with the required flow code in its header comment.
#
# Expected: MuMain/cmake/FindDotnetAOT.cmake contains:
#   - Flow code "VS1-NET-CMAKE-RID" in header comment block
#
# This ensures traceability from the source file back to the
# Value Stream flow code defined in the story metadata.

cmake_minimum_required(VERSION 3.25)

# MODULE_FILE is passed via -D from CTest invocation
if(NOT DEFINED MODULE_FILE)
    set(MODULE_FILE "${CMAKE_CURRENT_LIST_DIR}/../../cmake/FindDotnetAOT.cmake")
endif()

# --- Check 1: File exists ---
if(NOT EXISTS "${MODULE_FILE}")
    message(FATAL_ERROR "AC-STD-11: FindDotnetAOT.cmake does not exist: ${MODULE_FILE}")
endif()

file(READ "${MODULE_FILE}" MODULE_CONTENT)

# --- Check 2: Flow code VS1-NET-CMAKE-RID is present ---
string(FIND "${MODULE_CONTENT}" "VS1-NET-CMAKE-RID" _pos_flow_code)
if(_pos_flow_code EQUAL -1)
    message(FATAL_ERROR "AC-STD-11: Flow code 'VS1-NET-CMAKE-RID' not found in FindDotnetAOT.cmake -- required for AC-STD-11 traceability")
endif()

# --- Check 3: Flow code appears in a comment (starts within first 50 lines) ---
# Read beginning of file to verify it is in the header comment, not buried in logic
string(SUBSTRING "${MODULE_CONTENT}" 0 2000 _header_block)
string(FIND "${_header_block}" "VS1-NET-CMAKE-RID" _pos_in_header)
if(_pos_in_header EQUAL -1)
    message(FATAL_ERROR "AC-STD-11: Flow code 'VS1-NET-CMAKE-RID' not found in the first ~2000 chars of FindDotnetAOT.cmake -- it should be in the file header comment")
endif()

# --- Check 4: Story reference is also present (cross-reference) ---
string(FIND "${MODULE_CONTENT}" "3.1.1" _pos_story_ref)
string(FIND "${MODULE_CONTENT}" "3-1-1" _pos_story_ref2)
if(_pos_story_ref EQUAL -1 AND _pos_story_ref2 EQUAL -1)
    message(WARNING "AC-STD-11: Story reference '3.1.1' or '3-1-1' not found in FindDotnetAOT.cmake header -- recommended for full traceability")
endif()

message(STATUS "AC-STD-11: PASSED -- Flow code 'VS1-NET-CMAKE-RID' is present in FindDotnetAOT.cmake header")
