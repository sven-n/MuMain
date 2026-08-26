cmake_minimum_required(VERSION 3.25)

set(test_root "${CMAKE_CURRENT_LIST_DIR}/.tmp-copy-runtime-assets")
set(asset_source "${test_root}/source")
set(asset_output "${test_root}/output")

file(REMOVE_RECURSE "${test_root}")
file(MAKE_DIRECTORY
    "${asset_source}/Data"
    "${asset_source}/fonts"
    "${asset_output}/Data"
    "${asset_output}/fonts"
)
file(WRITE "${asset_source}/Data/Dec2.dat" "data")
file(WRITE "${asset_source}/fonts/DejaVuSans.ttf" "font")
file(WRITE "${asset_source}/MuMainIcon.png" "icon")
file(WRITE "${asset_source}/config.ini.template" "config")
file(WRITE "${asset_source}/legacy.dll" "dll")
file(WRITE "${asset_output}/Data/stale.dat" "stale")
file(WRITE "${asset_output}/fonts/stale.ttf" "stale")

execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_ASSET_SOURCE=${asset_source}"
        "-DMU_ASSET_OUTPUT=${asset_output}"
        "-DMU_COPY_RUNTIME_ASSETS=OFF"
        -P "${CMAKE_CURRENT_LIST_DIR}/../cmake/CopyRuntimeAssets.cmake"
    RESULT_VARIABLE copy_result
)

set(errors)
if(NOT copy_result EQUAL 0)
    list(APPEND errors "copy script exited ${copy_result}")
endif()
foreach(required_file IN ITEMS MuMainIcon.png config.ini.template)
    if(NOT EXISTS "${asset_output}/${required_file}")
        list(APPEND errors "missing retained runtime file: ${required_file}")
    endif()
endforeach()
foreach(forbidden_path IN ITEMS Data fonts legacy.dll)
    if(EXISTS "${asset_output}/${forbidden_path}")
        list(APPEND errors "unexpected no-data output: ${forbidden_path}")
    endif()
endforeach()

file(REMOVE_RECURSE "${test_root}")

if(errors)
    list(JOIN errors "\n- " error_text)
    message(FATAL_ERROR "CopyRuntimeAssets no-data contract failed:\n- ${error_text}")
endif()

message(STATUS "CopyRuntimeAssets no-data contract: OK")
