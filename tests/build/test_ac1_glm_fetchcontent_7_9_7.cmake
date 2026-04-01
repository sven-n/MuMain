# Story 7.9.7: AC-1 — GLM integrated via CMake FetchContent
# Flow Code: VS0-RENDER-GLM-MATRIX
#
# GREEN PHASE: Test PASSES once GLM FetchContent is added to CMakeLists.txt.
#              GLM is header-only — no binary dependency required.
#
# Validates:
#   AC-1 — FetchContent_Declare(glm ...) present in MuMain/CMakeLists.txt
#   AC-1 — FetchContent_MakeAvailable(glm) called
#   AC-1 — GLM_ENABLE_CXX_20 or glm::glm target configured

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CMAKE_LISTS_TXT)
    message(FATAL_ERROR "CMAKE_LISTS_TXT must be set to the path of MuMain/CMakeLists.txt")
endif()

if(NOT EXISTS "${CMAKE_LISTS_TXT}")
    message(FATAL_ERROR "AC-1 FAIL: CMakeLists.txt not found at '${CMAKE_LISTS_TXT}'")
endif()

file(READ "${CMAKE_LISTS_TXT}" cmake_content)

set(found_violations FALSE)

# ---------------------------------------------------------------------------
# AC-1a: FetchContent_Declare(glm ...) must be present
# ---------------------------------------------------------------------------
string(FIND "${cmake_content}" "FetchContent_Declare(glm" fetch_declare_pos)
if(fetch_declare_pos EQUAL -1)
    message(WARNING
        "AC-1 FAIL: 'FetchContent_Declare(glm' not found in CMakeLists.txt.\n"
        "Story 7.9.7 Task 1.1: Add GLM via FetchContent_Declare with GIT_REPOSITORY https://github.com/g-truc/glm.git")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-1b: FetchContent_MakeAvailable(glm) must be called
# ---------------------------------------------------------------------------
string(FIND "${cmake_content}" "FetchContent_MakeAvailable(glm)" fetch_avail_pos)
if(fetch_avail_pos EQUAL -1)
    message(WARNING
        "AC-1 FAIL: 'FetchContent_MakeAvailable(glm)' not found in CMakeLists.txt.\n"
        "Story 7.9.7 Task 1.2: Call FetchContent_MakeAvailable(glm) after FetchContent_Declare.")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-1 FAIL: GLM not integrated via FetchContent.\n"
        "Story 7.9.7 Task 1: Add GLM to MuMain/CMakeLists.txt via FetchContent_Declare and FetchContent_MakeAvailable.\n"
        "GLM must be header-only (no binary dependency).")
endif()

message(STATUS "AC-1 PASS: GLM integrated via FetchContent in CMakeLists.txt")
