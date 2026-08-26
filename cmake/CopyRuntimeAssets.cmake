if(NOT DEFINED MU_ASSET_SOURCE OR NOT IS_DIRECTORY "${MU_ASSET_SOURCE}")
    message(FATAL_ERROR "MU_ASSET_SOURCE must name an existing directory")
endif()
if(NOT DEFINED MU_ASSET_OUTPUT OR MU_ASSET_OUTPUT STREQUAL "")
    message(FATAL_ERROR "MU_ASSET_OUTPUT is required")
endif()
if(NOT DEFINED MU_COPY_RUNTIME_ASSETS)
    set(MU_COPY_RUNTIME_ASSETS ON)
endif()

if(NOT MU_COPY_RUNTIME_ASSETS)
    file(REMOVE_RECURSE "${MU_ASSET_OUTPUT}/Data" "${MU_ASSET_OUTPUT}/fonts")
    file(COPY "${MU_ASSET_SOURCE}/"
        DESTINATION "${MU_ASSET_OUTPUT}"
        PATTERN "*.dll" EXCLUDE
        PATTERN "Data" EXCLUDE
        PATTERN "fonts" EXCLUDE
    )
    return()
endif()

file(GLOB_RECURSE asset_dlls
    LIST_DIRECTORIES FALSE
    RELATIVE "${MU_ASSET_SOURCE}"
    "${MU_ASSET_SOURCE}/*.dll"
)
foreach(asset_dll IN LISTS asset_dlls)
    file(REMOVE "${MU_ASSET_OUTPUT}/${asset_dll}")
endforeach()

file(COPY "${MU_ASSET_SOURCE}/"
    DESTINATION "${MU_ASSET_OUTPUT}"
    PATTERN "*.dll" EXCLUDE
)
