# AC-3: Validate SDL3 link visibility -- MUPlatform and MURenderFX link SDL3;
#        MUGame, Main, MUCore etc. do NOT link SDL3 directly
#
# Story: 1.3.1 - SDL3 Dependency Integration
# RED phase: This test FAILS until SDL3 link targets are correctly configured.
#
# Expected in MuMain/src/CMakeLists.txt:
#   - target_link_libraries(MUPlatform PRIVATE SDL3::SDL3...) present
#   - target_link_libraries(MURenderFX PRIVATE SDL3::SDL3...) present
#   - NO target_link_libraries(MUGame ... SDL3...) anywhere
#   - NO target_link_libraries(Main ... SDL3...) anywhere
#   - NO target_link_libraries(MUCore ... SDL3...) anywhere
#   - SDL3 NOT added to MUCommon INTERFACE

cmake_minimum_required(VERSION 3.25)

# CMAKELISTS_FILE is passed via -D from the CTest invocation
if(NOT DEFINED CMAKELISTS_FILE)
    message(FATAL_ERROR "AC-3: CMAKELISTS_FILE variable not defined")
endif()

if(NOT EXISTS "${CMAKELISTS_FILE}")
    message(FATAL_ERROR "AC-3: CMakeLists.txt does not exist: ${CMAKELISTS_FILE}")
endif()

file(READ "${CMAKELISTS_FILE}" CMAKE_CONTENT)

# --- Check 1: MUPlatform links SDL3 PRIVATELY ---
# Match target_link_libraries(MUPlatform ... SDL3::SDL3...) with PRIVATE
string(REGEX MATCH "target_link_libraries[\\(\\s]*MUPlatform[\\s]+PRIVATE[^\\)]*SDL3" _match_platform "${CMAKE_CONTENT}")
if(NOT _match_platform)
    message(FATAL_ERROR "AC-3: MUPlatform does not link SDL3 with PRIVATE visibility")
endif()

# --- Check 2: MURenderFX links SDL3 PRIVATELY ---
string(REGEX MATCH "target_link_libraries[\\(\\s]*MURenderFX[\\s]+PRIVATE[^\\)]*SDL3" _match_renderfx "${CMAKE_CONTENT}")
if(NOT _match_renderfx)
    message(FATAL_ERROR "AC-3: MURenderFX does not link SDL3 with PRIVATE visibility")
endif()

# --- Check 3: MUGame does NOT link SDL3 ---
string(REGEX MATCH "target_link_libraries[\\(\\s]*MUGame[^\\)]*SDL3" _match_mugame "${CMAKE_CONTENT}")
if(_match_mugame)
    message(FATAL_ERROR "AC-3: MUGame MUST NOT link SDL3 directly -- found: ${_match_mugame}")
endif()

# --- Check 4: Main does NOT link SDL3 ---
string(REGEX MATCH "target_link_libraries[\\(\\s]*Main[^\\)]*SDL3" _match_main "${CMAKE_CONTENT}")
if(_match_main)
    message(FATAL_ERROR "AC-3: Main target MUST NOT link SDL3 directly -- found: ${_match_main}")
endif()

# --- Check 5: MUCore does NOT link SDL3 ---
string(REGEX MATCH "target_link_libraries[\\(\\s]*MUCore[^\\)]*SDL3" _match_mucore "${CMAKE_CONTENT}")
if(_match_mucore)
    message(FATAL_ERROR "AC-3: MUCore MUST NOT link SDL3 directly -- found: ${_match_mucore}")
endif()

# --- Check 6: MUCommon INTERFACE does NOT propagate SDL3 ---
string(REGEX MATCH "target_link_libraries[\\(\\s]*MUCommon[^\\)]*SDL3" _match_mucommon "${CMAKE_CONTENT}")
if(_match_mucommon)
    message(FATAL_ERROR "AC-3: MUCommon MUST NOT link SDL3 -- it would propagate to all downstream targets")
endif()

# --- Check 7: SDL3 link uses PRIVATE (not PUBLIC or INTERFACE) ---
string(REGEX MATCH "target_link_libraries[^\\)]*PUBLIC[^\\)]*SDL3" _match_public "${CMAKE_CONTENT}")
if(_match_public)
    message(FATAL_ERROR "AC-3: SDL3 must be linked with PRIVATE visibility, not PUBLIC -- found: ${_match_public}")
endif()

string(REGEX MATCH "target_link_libraries[^\\)]*INTERFACE[^\\)]*SDL3" _match_interface "${CMAKE_CONTENT}")
if(_match_interface)
    message(FATAL_ERROR "AC-3: SDL3 must be linked with PRIVATE visibility, not INTERFACE -- found: ${_match_interface}")
endif()

message(STATUS "AC-3: PASSED -- SDL3 link visibility is correct (MUPlatform + MURenderFX PRIVATE only)")
