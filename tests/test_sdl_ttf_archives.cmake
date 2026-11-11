cmake_minimum_required(VERSION 3.25)

# SDL_ttf comes as its release archive, which holds none of the four libraries SDL_ttf vendors (SDLTTF_VENDORED).
# SDL3_ttf's patch step unpacks each from an archive of its own into external/; without one, the configure stops in
# SDL_ttf ("Could not find CMakeLists.txt for harfbuzz in external/harfbuzz").

# The commits SDL_ttf's release pins the four libraries to, from a clone of libsdl-org/SDL_ttf:
#   git ls-tree release-3.2.2 external/
# A library's commit can change while its version stays (SDL_ttf's main moved HarfBuzz to another 8.5.0 commit), so
# sdl_ttf_vendored_versions can't see a stale one. An SDL_ttf update has to refresh this list as well.
set(sdl_ttf_release release-3.2.2)
set(freetype_commit 9973564cfa63763a3e4ac67c09147899539b1e07)
set(harfbuzz_commit 564bf9818a18709776856533829c0c04950773d6)
set(plutosvg_commit 2983eb6919feea272d793bc386384e3f5b97b03c)
set(plutovg_commit 3e6f922f453da1c9e7d1d7f66cac1d9724a18b47)

if(NOT DEFINED MU_ROOT_CMAKE)
    message(FATAL_ERROR "MU_ROOT_CMAKE is required")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/FetchContentDeclare.cmake")
mu_read_fetchcontent_declare("${MU_ROOT_CMAKE}" SDL3_ttf cmake_code declares declare_at)
if(NOT declares MATCHES "URL[ \t]+https://[^ \t\r\n]+\\.tar\\.gz"
        OR NOT declares MATCHES "URL_HASH[ \t]+SHA256=[0-9a-f]+")
    message(FATAL_ERROR "SDL3_ttf must come as a release archive checked by its SHA-256: ${declares}")
endif()
if(NOT declares MATCHES "/releases/download/${sdl_ttf_release}/")
    message(FATAL_ERROR
        "SDL3_ttf no longer comes from ${sdl_ttf_release}. Refresh the four commits at the top of "
        "${CMAKE_CURRENT_LIST_FILE} for the new release (git ls-tree <tag> external/), and the four archives and "
        "hashes in FetchContent_Declare(SDL3_ttf).")
endif()
# archive_extract_times_contract checks that these archives unpack with the time of unpacking.
set(vendored_setting "set\\([ \t]*SDLTTF_VENDORED[ \t]+ON[ \t]+CACHE[ \t]+BOOL[ \t]+\"[^\"]*\"[ \t]+FORCE[ \t]*\\)")
if(NOT cmake_code MATCHES "${vendored_setting}")
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

# Every patch step starts from SDL_ttf's own archive, which the download step keeps. Unpacked again first, it undoes
# the patches of an earlier run, so an edited patch doesn't land on top of its previous version.
if(NOT declares MATCHES "URL[ \t]+https://[^ \t\r\n]+/([^/ \t\r\n]+\\.tar\\.gz)")
    message(FATAL_ERROR "SDL3_ttf's URL must name a .tar.gz archive: ${declares}")
endif()
set(release_archive "${CMAKE_MATCH_1}")
string(REPLACE "." "\\." release_archive_pattern "${release_archive}")
list(GET commands 0 first_command)
if(NOT first_command MATCHES "\"-DARCHIVE=<DOWNLOAD_DIR>/${release_archive_pattern}\""
        OR NOT first_command MATCHES "\"-DDESTINATION=<SOURCE_DIR>\""
        OR NOT first_command MATCHES "UnpackArchive\\.cmake")
    message(FATAL_ERROR
        "The first patch command of FetchContent_Declare(SDL3_ttf) must unpack SDL_ttf's own archive again "
        "(-DARCHIVE=<DOWNLOAD_DIR>/${release_archive} -DDESTINATION=<SOURCE_DIR> -P cmake/UnpackArchive.cmake): "
        "without it, a patch step that runs again applies the patches on top of the previous ones. It is: "
        "${first_command}")
endif()

# A patch's SHA-256 in its command makes an edited patch run the patch step again.
foreach(command IN LISTS commands)
    if(command MATCHES "ApplyGitPatch\\.cmake" AND NOT command MATCHES "\"-DPATCH_SHA256=\\$\\{[^}]+\\}\"")
        message(FATAL_ERROR
            "Each patch command must pass its patch's SHA-256 (\"-DPATCH_SHA256=\${...}\"), so an edited patch runs "
            "the patch step again: ${command}")
    endif()
endforeach()

foreach(library IN ITEMS freetype harfbuzz plutosvg plutovg)
    set(unpack_index -1)
    set(index 0)
    foreach(command IN LISTS commands)
        if(command MATCHES "\"-DDESTINATION=<SOURCE_DIR>/external/${library}\""
                AND command MATCHES "UnpackArchive\\.cmake")
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
    set(commit "${CMAKE_MATCH_1}")
    string(LENGTH "${commit}" commit_length)
    if(NOT commit_length EQUAL 40)
        message(FATAL_ERROR "${library}'s archive must name a full 40-character commit, not ${commit}")
    endif()
    if(NOT commit STREQUAL "${${library}_commit}")
        message(FATAL_ERROR
            "${library}'s archive must be of ${${library}_commit}, the commit ${sdl_ttf_release} pins it to, not of "
            "${commit}")
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
