cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED REPO_ROOT)
    message(FATAL_ERROR "REPO_ROOT must point to the repository root")
endif()

set(ROOT_CMAKE_FILE "${REPO_ROOT}/CMakeLists.txt")
set(SOURCE_CMAKE_FILE "${REPO_ROOT}/src/CMakeLists.txt")
file(READ "${ROOT_CMAKE_FILE}" root_cmake)
file(READ "${SOURCE_CMAKE_FILE}" source_cmake)

set(expected_miniaudio_url
    "https://github.com/mackron/miniaudio/archive/9634bedb5b5a2ca38c1ee7108a9358a4e233f14d.tar.gz")
set(expected_miniaudio_hash
    "SHA256=1a3a79b80fc6f0b0cc155e28b954a598e0ddfa2db64e2afa8466be88c476fa55")

foreach(required_root_text IN ITEMS
        "FetchContent_Declare(miniaudio"
        "${expected_miniaudio_url}"
        "${expected_miniaudio_hash}"
        "SOURCE_SUBDIR _fetchcontent_only"
        "FetchContent_MakeAvailable(miniaudio)")
    string(FIND "${root_cmake}" "${required_root_text}" required_root_text_index)
    if(required_root_text_index EQUAL -1)
        message(FATAL_ERROR "Root CMake must contain: ${required_root_text}")
    endif()
endforeach()

string(CONCAT miniaudio_source_dir "$" "{miniaudio_SOURCE_DIR}")
foreach(required_include IN ITEMS
        "\"${miniaudio_source_dir}\""
        "\"${miniaudio_source_dir}/extras\"")
    string(FIND "${source_cmake}" "${required_include}" required_include_index)
    if(required_include_index EQUAL -1)
        message(FATAL_ERROR "MuClient must include fetched path: ${required_include}")
    endif()
endforeach()

string(FIND "${source_cmake}" "dependencies/miniaudio" vendored_include_index)
if(NOT vendored_include_index EQUAL -1)
    message(FATAL_ERROR "MuClient must not include the vendored miniaudio directory")
endif()

foreach(vendored_file IN ITEMS
        "${REPO_ROOT}/src/dependencies/miniaudio/miniaudio.h"
        "${REPO_ROOT}/src/dependencies/miniaudio/stb_vorbis.c")
    if(EXISTS "${vendored_file}")
        message(FATAL_ERROR "Vendored dependency must be removed: ${vendored_file}")
    endif()
endforeach()
