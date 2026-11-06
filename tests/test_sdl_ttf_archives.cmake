cmake_minimum_required(VERSION 3.25)

# SDL_ttf comes as its release archive, which holds none of the four libraries SDL_ttf vendors (SDLTTF_VENDORED).
# SDL3_ttf's patch step unpacks each from an archive of its own into external/; without one, the configure stops in
# SDL_ttf ("Could not find CMakeLists.txt for harfbuzz in external/harfbuzz").

if(NOT DEFINED MU_ROOT_CMAKE)
    message(FATAL_ERROR "MU_ROOT_CMAKE is required")
endif()

file(READ "${MU_ROOT_CMAKE}" cmake_source)
string(REGEX REPLACE "#[^\r\n]*" "" cmake_code "${cmake_source}")

string(REGEX MATCHALL "FetchContent_Declare\\([ \t\r\n]*SDL3_ttf[^)]*\\)" declares "${cmake_code}")
list(LENGTH declares declare_count)
if(NOT declare_count EQUAL 1)
    message(FATAL_ERROR "Expected one FetchContent_Declare(SDL3_ttf ...) in ${MU_ROOT_CMAKE}, found ${declare_count}")
endif()
if(NOT declares MATCHES "URL[ \t]+https://[^ \t\r\n]+\\.tar\\.gz" OR NOT declares MATCHES "URL_HASH[ \t]+SHA256=[0-9a-f]+")
    message(FATAL_ERROR "SDL3_ttf must come as a release archive checked by its SHA-256: ${declares}")
endif()
if(NOT cmake_code MATCHES "set\\([ \t]*SDLTTF_VENDORED[ \t]+ON[ \t]+CACHE[ \t]+BOOL[ \t]+\"[^\"]*\"[ \t]+FORCE[ \t]*\\)")
    message(FATAL_ERROR "${MU_ROOT_CMAKE} must set SDLTTF_VENDORED ON with CACHE BOOL \"\" FORCE: the archives below "
        "are SDL_ttf's vendored libraries")
endif()

foreach(library IN ITEMS freetype harfbuzz plutosvg plutovg)
    string(REGEX MATCH
        "\"-DURL=https://[^\"]+/archive/[0-9a-f]+\\.tar\\.gz\"[ \t\r\n]+\"-DSHA256=[0-9a-f]+\"[ \t\r\n]+\"-DDESTINATION=<SOURCE_DIR>/external/${library}\"[ \t\r\n]+-P[ \t\r\n]+\"[^\"]*/cmake/UnpackArchive\\.cmake\""
        unpack "${declares}")
    if(NOT unpack)
        message(FATAL_ERROR
            "FetchContent_Declare(SDL3_ttf) must unpack ${library} into <SOURCE_DIR>/external/${library} with "
            "cmake/UnpackArchive.cmake, from an archive of the commit SDL_ttf pins (-DURL=.../archive/<commit>.tar.gz) "
            "checked by its SHA-256 (-DSHA256=...): SDL_ttf's release archive leaves it out.")
    endif()
    string(FIND "${declares}" "${unpack}" unpack_at)
    string(FIND "${declares}" "ApplyGitPatch.cmake" first_patch_at)
    if(first_patch_at GREATER_EQUAL 0 AND unpack_at GREATER first_patch_at)
        message(FATAL_ERROR
            "FetchContent_Declare(SDL3_ttf) must unpack ${library} before the first patch (ApplyGitPatch.cmake)")
    endif()
endforeach()
