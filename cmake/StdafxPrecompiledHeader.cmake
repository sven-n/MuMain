# Helpers for stdafx.h as a precompiled header (issue #971), for the game library (src/CMakeLists.txt) and the tests
# that link it (tests/CMakeLists.txt).
include_guard(GLOBAL)

# mu_skip_pch_without_stdafx(<target> <source>...)
#
# A source of <target> that does not include stdafx.h itself (as "stdafx.h" or "App/stdafx.h") compiles without the
# precompiled header, as before. The check reads the files at configure time, so a file that adds or drops the include
# after that still builds, only without or with the header until the next configure. A relative source path is taken
# from <target>'s folder.
function(mu_skip_pch_without_stdafx target)
    get_target_property(source_dir ${target} SOURCE_DIR)
    foreach(source IN LISTS ARGN)
        cmake_path(ABSOLUTE_PATH source BASE_DIRECTORY "${source_dir}" OUTPUT_VARIABLE path)
        file(STRINGS "${path}" stdafx_include REGEX "^[ \t]*#[ \t]*include[ \t]+\"(App/)?stdafx\\.h\"" LIMIT_COUNT 1)
        if(NOT stdafx_include)
            set_source_files_properties("${path}" TARGET_DIRECTORY ${target} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
        endif()
    endforeach()
endfunction()

# mu_allow_gcc13_stringop_overflow(<target> <source>...)
#
# GCC 13 reports -Wstringop-overflow for some string concatenations ("..." + std::to_string(n) + "...",
# std::string(300, 'x') + ".json") when the file uses the precompiled header, a false positive; GCC 15 does not, 14 was
# not checked. It stays a warning in the given sources of <target>, so a local build with MU_WARNINGS_AS_ERRORS=ON still
# builds on Ubuntu 24.04. A relative source path is taken from <target>'s folder.
function(mu_allow_gcc13_stringop_overflow target)
    get_target_property(source_dir ${target} SOURCE_DIR)
    set(paths "")
    foreach(source IN LISTS ARGN)
        cmake_path(ABSOLUTE_PATH source BASE_DIRECTORY "${source_dir}" OUTPUT_VARIABLE path)
        list(APPEND paths "${path}")
    endforeach()
    set_property(SOURCE ${paths} TARGET_DIRECTORY ${target} APPEND PROPERTY COMPILE_OPTIONS
        "$<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,15>>:-Wno-error=stringop-overflow>")
endfunction()
