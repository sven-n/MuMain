# AC-1: Validate SDL3 is declared via FetchContent with a pinned version
#
# Story: 1.3.1 - SDL3 Dependency Integration
# RED phase: This test FAILS until SDL3 FetchContent is added to CMakeLists.txt.
#
# Expected: MuMain/src/CMakeLists.txt contains:
#   - FetchContent_Declare(SDL3 ...)
#   - A pinned GIT_TAG (release-3.x.y format, NOT "main" or "HEAD")
#   - GIT_REPOSITORY pointing to libsdl-org/SDL
#   - MU_ENABLE_SDL3 option for CI gating

cmake_minimum_required(VERSION 3.25)

# CMAKELISTS_FILE is passed via -D from the CTest invocation
if(NOT DEFINED CMAKELISTS_FILE)
    message(FATAL_ERROR "AC-1: CMAKELISTS_FILE variable not defined")
endif()

# --- Check 1: File exists ---
if(NOT EXISTS "${CMAKELISTS_FILE}")
    message(FATAL_ERROR "AC-1: CMakeLists.txt does not exist: ${CMAKELISTS_FILE}")
endif()

file(READ "${CMAKELISTS_FILE}" CMAKE_CONTENT)

# --- Check 2: FetchContent_Declare for SDL3 ---
string(FIND "${CMAKE_CONTENT}" "FetchContent_Declare" _pos_fetch_declare)
if(_pos_fetch_declare EQUAL -1)
    message(FATAL_ERROR "AC-1: CMakeLists.txt missing FetchContent_Declare")
endif()

string(REGEX MATCH "FetchContent_Declare[\\(\\s]*SDL3" _match_sdl3_declare "${CMAKE_CONTENT}")
if(NOT _match_sdl3_declare)
    # Try multiline match with newline
    string(FIND "${CMAKE_CONTENT}" "FetchContent_Declare(" _pos_fc)
    if(_pos_fc EQUAL -1)
        message(FATAL_ERROR "AC-1: FetchContent_Declare not found for SDL3")
    endif()
    # Check that SDL3 appears near FetchContent_Declare
    string(SUBSTRING "${CMAKE_CONTENT}" ${_pos_fc} 200 _fc_block)
    string(FIND "${_fc_block}" "SDL3" _pos_sdl3_in_block)
    if(_pos_sdl3_in_block EQUAL -1)
        message(FATAL_ERROR "AC-1: FetchContent_Declare does not reference SDL3")
    endif()
endif()

# --- Check 3: GIT_REPOSITORY points to SDL ---
string(REGEX MATCH "GIT_REPOSITORY[\\s]+https://github.com/libsdl-org/SDL" _match_repo "${CMAKE_CONTENT}")
if(NOT _match_repo)
    message(FATAL_ERROR "AC-1: GIT_REPOSITORY does not point to libsdl-org/SDL")
endif()

# --- Check 4: GIT_TAG is pinned (release-3.x.y format, NOT main/HEAD) ---
string(REGEX MATCH "GIT_TAG[\\s]+([a-zA-Z0-9._-]+)" _match_tag "${CMAKE_CONTENT}")
if(NOT _match_tag)
    message(FATAL_ERROR "AC-1: GIT_TAG not found in FetchContent_Declare")
endif()

# Extract the tag value
string(REGEX REPLACE "GIT_TAG[\\s]+" "" _tag_value "${_match_tag}")

# Reject floating references
if(_tag_value STREQUAL "main" OR _tag_value STREQUAL "HEAD" OR _tag_value STREQUAL "master")
    message(FATAL_ERROR "AC-1: GIT_TAG is '${_tag_value}' -- must be a pinned release tag (e.g., release-3.2.0), not a floating reference")
endif()

# Verify it looks like a release tag (release-3.x.y or similar versioned pattern)
string(REGEX MATCH "release-3\\.[0-9]" _match_release "${_tag_value}")
if(NOT _match_release)
    # Also accept plain version tags like v3.x.y or 3.x.y
    string(REGEX MATCH "^[v]?3\\.[0-9]" _match_version "${_tag_value}")
    if(NOT _match_version)
        message(FATAL_ERROR "AC-1: GIT_TAG '${_tag_value}' does not match expected SDL3 release pattern (release-3.x.y)")
    endif()
endif()

# --- Check 5: MU_ENABLE_SDL3 option exists ---
string(FIND "${CMAKE_CONTENT}" "MU_ENABLE_SDL3" _pos_enable_option)
if(_pos_enable_option EQUAL -1)
    message(FATAL_ERROR "AC-1: MU_ENABLE_SDL3 option not found in CMakeLists.txt")
endif()

# --- Check 6: FetchContent_MakeAvailable(SDL3) ---
string(FIND "${CMAKE_CONTENT}" "FetchContent_MakeAvailable" _pos_make_avail)
if(_pos_make_avail EQUAL -1)
    message(FATAL_ERROR "AC-1: FetchContent_MakeAvailable not found")
endif()

message(STATUS "AC-1: PASSED -- SDL3 FetchContent is declared with pinned version '${_tag_value}'")
