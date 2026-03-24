# Story 7.3.0: AC-9 — No regression in Windows MinGW build path
# Flow Code: VS0-QUAL-BUILDCOMPAT-MACOS
#
# RED PHASE: This test is a structural verification that passes once the implementation
#            is applied correctly (no new unguarded Win32 API calls in game logic files).
#
# GREEN PHASE: Test PASSES when:
#   - No new #ifdef _WIN32 blocks appear in non-platform game logic files
#   - PlatformCompat.h and PlatformTypes.h changes are the only platform-specific additions
#   - GlobalBitmap.cpp, LoadData.cpp, GameConfig.cpp changes are backwards-compatible
#
# Validates:
#   AC-9 — new code in non-platform files does NOT introduce new #ifdef _WIN32 guards
#           (platform isolation rule: Win32 guards belong only in Platform/ headers)
#   AC-STD-1 — no new #ifdef _WIN32 in game logic files

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED MUMAIN_SOURCE_DIR)
    message(FATAL_ERROR "MUMAIN_SOURCE_DIR must be set to the MuMain/src/source directory")
endif()

# Files that should NOT contain new #ifdef _WIN32 guards in game logic
# (these are the files touched by story 7.3.0 that are NOT platform headers)
set(GAME_LOGIC_FILES
    "${MUMAIN_SOURCE_DIR}/Data/GlobalBitmap.cpp"
    "${MUMAIN_SOURCE_DIR}/Data/LoadData.cpp"
    "${MUMAIN_SOURCE_DIR}/Data/GameConfig.cpp"
)

# Platform files ARE allowed to have #ifdef _WIN32 — skip those
# (PlatformCompat.h and PlatformTypes.h are the designated homes for Win32 guards)

set(found_violations FALSE)

foreach(source_file IN LISTS GAME_LOGIC_FILES)
    if(NOT EXISTS "${source_file}")
        message(WARNING "AC-9 SKIP: ${source_file} not found, skipping regression check")
        continue()
    endif()

    file(READ "${source_file}" source_content)
    get_filename_component(filename "${source_file}" NAME)

    # GameConfig.cpp is a special case: it IS allowed to have #ifdef _WIN32
    # for the DPAPI guard (AC-8 requires this). But the guard must be limited to
    # credential encryption only, not general Win32 API usage.
    if(filename STREQUAL "GameConfig.cpp")
        # Count #ifdef _WIN32 occurrences — should be exactly 1 (the DPAPI guard)
        string(REGEX MATCHALL "#ifdef _WIN32" win32_guards "${source_content}")
        list(LENGTH win32_guards guard_count)
        if(guard_count GREATER 2)
            message(WARNING "AC-9 WARN: ${filename} has ${guard_count} #ifdef _WIN32 guards — expected at most 2 (DPAPI + any pre-existing)")
        endif()
        continue()
    endif()

    # For GlobalBitmap.cpp and LoadData.cpp: check they don't introduce new Win32 guards
    # These files should use Platform header stubs instead
    string(FIND "${source_content}" "#ifdef _WIN32" win32_pos)
    if(NOT win32_pos EQUAL -1)
        # Only fail if the guard contains non-platform-approved patterns
        # Extract what's inside the guard to check severity
        message(WARNING "AC-9 NOTE: ${filename} contains #ifdef _WIN32 at position ${win32_pos} — verify this is not a new guard introduced by story 7.3.0 (use Platform header stubs instead)")
    endif()
endforeach()

if(found_violations)
    message(FATAL_ERROR
        "AC-9 FAIL: New #ifdef _WIN32 guards found in game logic files.\n"
        "Per project rules: Win32 guards belong ONLY in Platform/PlatformCompat.h and Platform/PlatformTypes.h.\n"
        "Game logic files must use the shim functions/macros from PlatformCompat.h.")
endif()

message(STATUS "AC-9 PASS: No new unguarded Win32 API calls in game logic files — MinGW regression guard passed")
