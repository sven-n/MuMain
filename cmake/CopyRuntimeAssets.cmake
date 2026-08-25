if(NOT DEFINED MU_ASSET_SOURCE OR NOT IS_DIRECTORY "${MU_ASSET_SOURCE}")
    message(FATAL_ERROR "MU_ASSET_SOURCE must name an existing directory")
endif()
if(NOT DEFINED MU_ASSET_OUTPUT OR MU_ASSET_OUTPUT STREQUAL "")
    message(FATAL_ERROR "MU_ASSET_OUTPUT is required")
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
