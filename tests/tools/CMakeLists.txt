# Asset-tool tests. Registered only when the tools are built (MU_BUILD_ASSET_TOOLS).
if(NOT TARGET bmdconv_lib)
    return()
endif()

mu_add_test(
    NAME test_bmdconv
    SOURCES
        test_bmdconv.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS bmdconv_lib
)
target_compile_definitions(test_bmdconv PRIVATE
    "MU_TEST_DATA_DIR=\"${CMAKE_SOURCE_DIR}/src/bin/Data/\""
    "MU_TEST_SCRATCH_DIR=\"${CMAKE_CURRENT_BINARY_DIR}/scratch/\""
)
