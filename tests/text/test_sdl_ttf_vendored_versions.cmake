cmake_minimum_required(VERSION 3.25)

# The root CMakeLists.txt pins the four libraries SDL_ttf vendors apart from SDL_ttf itself. SDL_ttf's .gitmodules
# names the branch each of them follows (e.g. 8.5.0-SDL for HarfBuzz), so a new SDL_ttf whose libraries were not
# updated with it shows as a library whose version no longer matches its branch.

if(NOT DEFINED SDL3_TTF_SOURCE)
    message(FATAL_ERROR "SDL3_TTF_SOURCE must be set")
endif()
set(gitmodules_file "${SDL3_TTF_SOURCE}/.gitmodules")
if(NOT EXISTS "${gitmodules_file}")
    message(FATAL_ERROR "required file is missing: ${gitmodules_file}")
endif()
file(READ "${gitmodules_file}" gitmodules)

# version_pattern holds NAME where each of the three version parts' names goes; its group is the part's number.
function(check_vendored_version library version_file version_pattern major_name minor_name patch_name)
    if(NOT gitmodules MATCHES "\\[submodule \"external/${library}\"\\][^[]*branch[ \t]*=[ \t]*([^\r\n]+)")
        message(FATAL_ERROR "${gitmodules_file} names no branch for external/${library}")
    endif()
    set(branch "${CMAKE_MATCH_1}")
    if(NOT branch MATCHES "([0-9]+)[-.]([0-9]+)[-.]([0-9]+)")
        message(FATAL_ERROR "The branch ${branch} of external/${library} names no version")
    endif()
    set(branch_version "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")

    set(source_file "${SDL3_TTF_SOURCE}/external/${library}/${version_file}")
    if(NOT EXISTS "${source_file}")
        message(FATAL_ERROR "required file is missing: ${source_file}")
    endif()
    file(READ "${source_file}" source)
    set(source_version "")
    foreach(part_name IN ITEMS ${major_name} ${minor_name} ${patch_name})
        string(REPLACE "NAME" "${part_name}" part_pattern "${version_pattern}")
        if(NOT source MATCHES "${part_pattern}")
            message(FATAL_ERROR "${source_file} does not state ${part_name}")
        endif()
        list(APPEND source_version "${CMAKE_MATCH_1}")
    endforeach()
    list(JOIN source_version "." source_version)

    if(NOT source_version STREQUAL branch_version)
        message(FATAL_ERROR
            "external/${library} is version ${source_version}, but SDL_ttf follows its branch ${branch}: update its "
            "commit and SHA-256 in the root CMakeLists.txt to the commit SDL_ttf pins (git ls-tree <tag> external/)")
    endif()
endfunction()

check_vendored_version(freetype include/freetype/freetype.h "#define NAME[ \t]+([0-9]+)"
    FREETYPE_MAJOR FREETYPE_MINOR FREETYPE_PATCH)
check_vendored_version(harfbuzz src/hb-version.h "#define NAME[ \t]+([0-9]+)"
    HB_VERSION_MAJOR HB_VERSION_MINOR HB_VERSION_MICRO)
check_vendored_version(plutosvg CMakeLists.txt "set\\(NAME[ \t]+([0-9]+)\\)"
    PLUTOSVG_VERSION_MAJOR PLUTOSVG_VERSION_MINOR PLUTOSVG_VERSION_MICRO)
check_vendored_version(plutovg CMakeLists.txt "set\\(NAME[ \t]+([0-9]+)\\)"
    PLUTOVG_VERSION_MAJOR PLUTOVG_VERSION_MINOR PLUTOVG_VERSION_MICRO)
