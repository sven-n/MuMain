cmake_minimum_required(VERSION 3.25)

# SDL_image comes as its release archive, which holds none of the libraries SDL_image can vendor. Each setting below
# keeps SDL_image off one of them; without it, the configure stops in SDL_image ("Could not find CMakeLists.txt for
# libjxl in external/libjxl").

if(NOT DEFINED MU_ROOT_CMAKE)
    message(FATAL_ERROR "MU_ROOT_CMAKE is required")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/FetchContentDeclare.cmake")
mu_read_fetchcontent_declare("${MU_ROOT_CMAKE}" SDL3_image cmake_code declares declare_at)
if(NOT declares MATCHES "URL[ \t]+https://[^ \t\r\n]+\\.tar\\.gz" OR NOT declares MATCHES "URL_HASH[ \t]+SHA256=[0-9a-f]+")
    message(FATAL_ERROR "SDL3_image must come as a release archive checked by its SHA-256: ${declares}")
endif()

# Setting, value, and the vendored libraries SDL_image would build without it.
set(settings
    "SDLIMAGE_AVIF|OFF|libavif, dav1d and aom"
    "SDLIMAGE_JXL|OFF|libjxl"
    "SDLIMAGE_TIF|OFF|libtiff"
    "SDLIMAGE_WEBP|OFF|libwebp"
    "SDLIMAGE_BACKEND_STB|ON|libpng, jpeg and zlib"
)
foreach(setting IN LISTS settings)
    string(REPLACE "|" ";" parts "${setting}")
    list(GET parts 0 name)
    list(GET parts 1 value)
    list(GET parts 2 libraries)
    string(REGEX MATCH "set\\([ \t]*${name}[ \t]+${value}[ \t]+CACHE[ \t]+BOOL[ \t]+\"[^\"]*\"[ \t]+FORCE[ \t]*\\)"
        found "${cmake_code}")
    string(FIND "${cmake_code}" "${found}" found_at)
    if(NOT found OR found_at GREATER declare_at)
        message(FATAL_ERROR
            "${MU_ROOT_CMAKE} must set ${name} ${value} with CACHE BOOL \"\" FORCE before FetchContent_Declare(SDL3_image): "
            "otherwise SDL_image builds the vendored ${libraries}, which its release archive leaves out. To build them, "
            "fetch SDL_image with them first (see the comment above FetchContent_Declare(SDL3_image)).")
    endif()
endforeach()
