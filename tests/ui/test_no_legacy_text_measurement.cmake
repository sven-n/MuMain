# Source inspection is deliberate here: this enforces a repository-wide ban,
# not a particular implementation shape. Rendering behavior stays covered by
# the text wrapping and SDL_ttf tests.
cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be set")
endif()

file(GLOB_RECURSE source_files
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
)

foreach(source_file IN LISTS source_files)
    file(RELATIVE_PATH relative_file "${SOURCE_DIR}" "${source_file}")
    if(relative_file MATCHES "^((App|Core)/Platform|ThirdParty)/")
        continue()
    endif()

    file(READ "${source_file}" source)
    string(REGEX REPLACE "//[^\n\r]*" "" source "${source}")
    string(REGEX REPLACE "/\\*([^*]|\\*+[^*/])*\\*+/" "" source "${source}")
    if(source MATCHES "(^|[^A-Za-z0-9_])(GetTextExtentPoint32(A|W)?|GetFontDC|GetFontBuffer)[ \t\r\n]*\\(")
        message(FATAL_ERROR "${relative_file} still uses legacy GDI text measurement")
    endif()

    if(source MATCHES "(^|[^A-Za-z0-9_])FontHeight([^A-Za-z0-9_]|$)")
        message(FATAL_ERROR "${relative_file} still uses the legacy global FontHeight")
    endif()

    if(relative_file STREQUAL "Render/Text/CUIRenderTextSDLTtf.cpp" AND
       source MATCHES "(^|[^A-Za-z0-9_])TTF_GetStringSize[ \\t\\r\\n]*\\(")
        message(FATAL_ERROR "${relative_file} still bypasses retained SDL_ttf text layout")
    endif()
endforeach()
