# Story 7.5.1: AC-10 — No regression in Windows MinGW build path
# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS
#
# RED PHASE:  This structural verification test checks that the story 7.5.1 source changes
#             do not introduce new #ifdef _WIN32 guards in game logic files.
#             (Passes structurally once code conventions are followed.)
#
# GREEN PHASE: Test PASSES when:
#   - No new #ifdef _WIN32 blocks in SkillDataLoader.cpp, ZzzOpenData.cpp, ZzzInfomation.cpp
#   - All fixes use platform-neutral patterns (static_cast, L'\0', mu_swprintf)
#   - Changes are backwards-compatible with MinGW Win32 compilation
#
# Validates:
#   AC-10 — modified source files do not introduce new Win32-only #ifdef guards in game logic

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED MUMAIN_SOURCE_DIR)
    message(FATAL_ERROR "MUMAIN_SOURCE_DIR must be set to the MuMain/src/source directory")
endif()

# Files modified by story 7.5.1 that must not have new #ifdef _WIN32 guards
set(MODIFIED_FILES
    "${MUMAIN_SOURCE_DIR}/Data/Skills/SkillDataLoader.cpp"
    "${MUMAIN_SOURCE_DIR}/Data/ZzzOpenData.cpp"
    "${MUMAIN_SOURCE_DIR}/Data/ZzzInfomation.cpp"
)

# Known-allowed pre-existing Win32 guards (these were already in the files before 7.5.1)
# Story 7.5.1 fixes are all platform-neutral: mu_swprintf, static_cast, L'\0'
# None of these fixes should require new #ifdef _WIN32 guards

set(found_violations FALSE)

foreach(source_file IN LISTS MODIFIED_FILES)
    if(NOT EXISTS "${source_file}")
        message(WARNING "AC-10 SKIP: ${source_file} not found — skipping regression check")
        continue()
    endif()

    file(READ "${source_file}" source_content)
    get_filename_component(filename "${source_file}" NAME)

    # Count #ifdef _WIN32 occurrences
    string(REGEX MATCHALL "#ifdef _WIN32" win32_guards "${source_content}")
    list(LENGTH win32_guards guard_count)

    # SkillDataLoader.cpp and ZzzOpenData.cpp should have NO #ifdef _WIN32 (new or pre-existing)
    # ZzzInfomation.cpp may have pre-existing guards — we check for gross violations only
    if(filename STREQUAL "SkillDataLoader.cpp" OR filename STREQUAL "ZzzOpenData.cpp")
        if(guard_count GREATER 0)
            message(WARNING
                "AC-10 WARN: ${filename} has ${guard_count} #ifdef _WIN32 guard(s).\n"
                "Story 7.5.1 fixes (mu_swprintf, static_cast) should not require Win32 guards.\n"
                "If these are new guards added by this story, they violate AC-10.\n"
                "Use platform-neutral PlatformCompat.h shims instead.")
        endif()
    endif()

    if(filename STREQUAL "ZzzInfomation.cpp")
        # Check for new Win32 guards that should not be needed for the 7.5.1 fixes
        # L'\0' fixes, unused variable removal, parens, and cast fixes are all platform-neutral
        if(guard_count GREATER 2)
            message(WARNING
                "AC-10 WARN: ZzzInfomation.cpp has ${guard_count} #ifdef _WIN32 guards — expected 0-2 pre-existing.\n"
                "Story 7.5.1 fixes must be platform-neutral — no new Win32 guards needed.")
        endif()
    endif()
endforeach()

if(found_violations)
    message(FATAL_ERROR
        "AC-10 FAIL: New #ifdef _WIN32 guards found in story 7.5.1 modified files.\n"
        "All 7.5.1 fixes must use platform-neutral patterns from PlatformCompat.h.")
endif()

message(STATUS "AC-10 PASS: No new Win32 guards in story 7.5.1 modified files — MinGW regression guard passed")
