# Negative policy scan: compatibility declarations keep raw IME calls
# buildable, so compilation alone cannot enforce the SDL text-input boundary.
cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be set")
endif()

set(ALLOWED_FILES
    "Core/Input/ImeInput.cpp"
    "Scenes/CharacterScene.cpp"
    "UI/Widgets/UIControls.cpp"
)

file(GLOB_RECURSE source_files
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
)

set(unexpected_files "")
foreach(source_file IN LISTS source_files)
    file(READ "${source_file}" source)
    string(REGEX REPLACE "//[^\n\r]*" "" source "${source}")
    string(REGEX REPLACE "/\\*([^*]|\\*+[^*/])*\\*+/" "" source "${source}")
    string(REGEX MATCH
        "(^|[^A-Za-z0-9_])(ImmGetContext|ImmSetConversionStatus|ImmReleaseContext|ImmGetConversionStatus|ImmSetCompositionWindow)[ \t\r\n]*\\("
        match "${source}")
    if(NOT match)
        continue()
    endif()

    file(RELATIVE_PATH relative_file "${SOURCE_DIR}" "${source_file}")
    if(relative_file MATCHES "^((App|Core)/Platform|ThirdParty)/")
        continue()
    endif()

    list(FIND ALLOWED_FILES "${relative_file}" allowed_index)
    if(allowed_index EQUAL -1)
        list(APPEND unexpected_files "${relative_file}")
    endif()
endforeach()

if(unexpected_files)
    list(JOIN unexpected_files "\n  " unexpected_list)
    message(FATAL_ERROR
        "New raw IME calls must use the SDL text-input abstraction:\n  ${unexpected_list}")
endif()
