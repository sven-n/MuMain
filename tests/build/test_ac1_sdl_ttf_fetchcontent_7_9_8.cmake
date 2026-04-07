# Story 7.9.8: AC-1 — SDL_ttf 3.2.2+ integrated via CMake FetchContent
# Flow Code: VS1-RENDER-FONT-SDLTTF
#
# RED PHASE: Test FAILS until SDL_ttf FetchContent is added to MuMain/CMakeLists.txt.
# GREEN PHASE: Test PASSES once FetchContent_Declare(SDL_ttf ...) and
#              FetchContent_MakeAvailable(SDL_ttf) are present and SDL3_ttf is linked.
#
# Validates:
#   AC-1 — FetchContent_Declare(SDL_ttf ...) present in MuMain/CMakeLists.txt
#   AC-1 — FetchContent_MakeAvailable(SDL_ttf) called
#   AC-1 — SDL3_ttf (or SDL3_ttf::SDL3_ttf) linked to a render target

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
# AC-1a: FetchContent_Declare(SDL_ttf ...) must be present
# ---------------------------------------------------------------------------
string(FIND "${cmake_content}" "FetchContent_Declare(SDL_ttf" fetch_declare_pos)
if(fetch_declare_pos EQUAL -1)
    # Also accept SDL3_ttf naming variant
    string(FIND "${cmake_content}" "FetchContent_Declare(SDL3_ttf" fetch_declare_alt_pos)
    if(fetch_declare_alt_pos EQUAL -1)
        message(WARNING
            "AC-1 FAIL: 'FetchContent_Declare(SDL_ttf' not found in CMakeLists.txt.\n"
            "Story 7.9.8 Task 1.1: Add SDL_ttf via FetchContent_Declare with "
            "GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git "
            "and GIT_TAG set to sdl3-ttf-3.2.2 or later.")
        set(found_violations TRUE)
    endif()
endif()

# ---------------------------------------------------------------------------
# AC-1b: FetchContent_MakeAvailable for SDL_ttf must be called
# ---------------------------------------------------------------------------
string(FIND "${cmake_content}" "FetchContent_MakeAvailable(SDL_ttf" fetch_avail_pos)
if(fetch_avail_pos EQUAL -1)
    string(FIND "${cmake_content}" "FetchContent_MakeAvailable(SDL3_ttf" fetch_avail_alt_pos)
    if(fetch_avail_alt_pos EQUAL -1)
        message(WARNING
            "AC-1 FAIL: 'FetchContent_MakeAvailable(SDL_ttf' not found in CMakeLists.txt.\n"
            "Story 7.9.8 Task 1.2: Call FetchContent_MakeAvailable(SDL_ttf) after FetchContent_Declare.")
        set(found_violations TRUE)
    endif()
endif()

# ---------------------------------------------------------------------------
# AC-1c: SDL3_ttf target must be referenced (linked to render target)
# ---------------------------------------------------------------------------
string(REGEX MATCH "SDL3_ttf|SDL3::SDL3_ttf" sdl_ttf_link_match "${cmake_content}")
if(NOT sdl_ttf_link_match)
    message(WARNING
        "AC-1 FAIL: No SDL3_ttf link target found in CMakeLists.txt.\n"
        "Story 7.9.8 Task 1.3: Add target_link_libraries(MURenderFX PRIVATE SDL3_ttf::SDL3_ttf) "
        "inside the MU_ENABLE_SDL3 block (parallel to SDL3::SDL3-static).")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-1 FAIL: SDL_ttf not integrated via FetchContent.\n"
        "Story 7.9.8 Task 1: Add SDL_ttf 3.2.2+ to MuMain/CMakeLists.txt via "
        "FetchContent_Declare and FetchContent_MakeAvailable, then link SDL3_ttf::SDL3_ttf to MURenderFX.\n"
        "Pattern to follow: SDL3 and GLM FetchContent blocks already in CMakeLists.txt.")
endif()

message(STATUS "AC-1 PASS: SDL_ttf integrated via FetchContent in CMakeLists.txt")
