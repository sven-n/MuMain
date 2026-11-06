cmake_minimum_required(VERSION 3.25)

# SDL_ttf comes as its release archive, which holds none of the four libraries SDL_ttf vendors (SDLTTF_VENDORED).
# SDL3_ttf's patch step unpacks each from an archive of its own into external/; without one, the configure stops in
# SDL_ttf ("Could not find CMakeLists.txt for harfbuzz in external/harfbuzz").

if(NOT DEFINED MU_ROOT_CMAKE)
    message(FATAL_ERROR "MU_ROOT_CMAKE is required")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/FetchContentDeclare.cmake")
mu_read_fetchcontent_declare("${MU_ROOT_CMAKE}" SDL3_ttf cmake_code declares declare_at)
if(NOT declares MATCHES "URL[ \t]+https://[^ \t\r\n]+\\.tar\\.gz" OR NOT declares MATCHES "URL_HASH[ \t]+SHA256=[0-9a-f]+")
    message(FATAL_ERROR "SDL3_ttf must come as a release archive checked by its SHA-256: ${declares}")
endif()
if(NOT cmake_code MATCHES "set\\([ \t]*SDLTTF_VENDORED[ \t]+ON[ \t]+CACHE[ \t]+BOOL[ \t]+\"[^\"]*\"[ \t]+FORCE[ \t]*\\)")
    message(FATAL_ERROR "${MU_ROOT_CMAKE} must set SDLTTF_VENDORED ON with CACHE BOOL \"\" FORCE: the archives below "
        "are SDL_ttf's vendored libraries")
endif()

# One entry per patch command, in the order they run.
string(REGEX REPLACE "(PATCH_)?COMMAND[ \t\r\n]" ";" commands "${declares}")
list(POP_FRONT commands)
set(first_patch_index -1)
set(index 0)
foreach(command IN LISTS commands)
    if(first_patch_index EQUAL -1 AND command MATCHES "ApplyGitPatch\\.cmake")
        set(first_patch_index ${index})
    endif()
    math(EXPR index "${index} + 1")
endforeach()

foreach(library IN ITEMS freetype harfbuzz plutosvg plutovg)
    set(unpack_index -1)
    set(index 0)
    foreach(command IN LISTS commands)
        if(command MATCHES "\"-DDESTINATION=<SOURCE_DIR>/external/${library}\"" AND command MATCHES "UnpackArchive\\.cmake")
            set(unpack_index ${index})
            set(unpack "${command}")
        endif()
        math(EXPR index "${index} + 1")
    endforeach()
    if(unpack_index EQUAL -1)
        message(FATAL_ERROR
            "FetchContent_Declare(SDL3_ttf) must unpack ${library} into <SOURCE_DIR>/external/${library} with "
            "cmake/UnpackArchive.cmake: SDL_ttf's release archive leaves it out.")
    endif()
    if(NOT unpack MATCHES "\"-DURL=https://[^\"]+/${library}/archive/([0-9a-f]+)\\.tar\\.gz\"")
        message(FATAL_ERROR
            "${library} must come as an archive of a commit of its own repository "
            "(-DURL=https://github.com/libsdl-org/${library}/archive/<commit>.tar.gz): ${unpack}")
    endif()
    string(LENGTH "${CMAKE_MATCH_1}" commit_length)
    if(NOT commit_length EQUAL 40)
        message(FATAL_ERROR "${library}'s archive must name a full 40-character commit, not ${CMAKE_MATCH_1}")
    endif()
    if(NOT unpack MATCHES "\"-DSHA256=([0-9a-f]+)\"")
        message(FATAL_ERROR "${library}'s archive must be checked by its SHA-256 (-DSHA256=...): ${unpack}")
    endif()
    string(LENGTH "${CMAKE_MATCH_1}" hash_length)
    if(NOT hash_length EQUAL 64)
        message(FATAL_ERROR "${library}'s SHA-256 must have 64 hex digits, not ${CMAKE_MATCH_1}")
    endif()
    if(first_patch_index GREATER_EQUAL 0 AND unpack_index GREATER first_patch_index)
        message(FATAL_ERROR
            "FetchContent_Declare(SDL3_ttf) must unpack ${library} before the first patch (ApplyGitPatch.cmake)")
    endif()
endforeach()
