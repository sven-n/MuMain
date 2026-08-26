cmake_minimum_required(VERSION 3.25)

set(required_assets
    fonts/DejaVuSans.ttf
    fonts/DejaVuSans-Bold.ttf
    fonts/DejaVuSans-LICENSE
    fonts/LiberationSans-Regular.ttf
    fonts/LiberationSans-Bold.ttf
    fonts/LiberationSans-LICENSE
    fonts/Cousine-Regular.ttf
    fonts/Cousine-LICENSE.txt
)
foreach(relative_path IN LISTS required_assets)
    set(source_path "${MU_ASSET_SOURCE}/${relative_path}")
    if(NOT EXISTS "${source_path}")
        message(FATAL_ERROR "missing bundled font asset: ${relative_path}")
    endif()
    file(SIZE "${source_path}" source_size)
    if(source_size EQUAL 0)
        message(FATAL_ERROR "empty bundled font asset: ${relative_path}")
    endif()
endforeach()

set(asset_output "${CMAKE_CURRENT_LIST_DIR}/.tmp-bundled-font-assets")
file(REMOVE_RECURSE "${asset_output}")
file(MAKE_DIRECTORY "${asset_output}")

execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_ASSET_SOURCE=${MU_ASSET_SOURCE}"
        "-DMU_ASSET_OUTPUT=${asset_output}"
        -DMU_COPY_RUNTIME_ASSETS=ON
        -P "${MU_COPY_SCRIPT}"
    RESULT_VARIABLE copy_result
)
if(NOT copy_result EQUAL 0)
    message(FATAL_ERROR "runtime asset copy exited ${copy_result}")
endif()

foreach(relative_path IN LISTS required_assets)
    if(NOT EXISTS "${asset_output}/${relative_path}")
        message(FATAL_ERROR "missing staged bundled font: ${relative_path}")
    endif()
endforeach()

file(REMOVE_RECURSE "${asset_output}")
message(STATUS "Bundled font assets: OK")
