add_test(
    NAME no_raw_getasynckeystate
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}/src/source
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std3_no_raw_getasynckeystate.cmake
)

add_test(
    NAME no_raw_win32_mouse
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}/src/source
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std3_no_raw_win32_mouse.cmake
)

add_test(
    NAME no_raw_imm_apis
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}/src/source
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std3_no_raw_imm.cmake
)

mu_add_test(
    NAME sdl_window_flags_tests
    SOURCES
        test_sdl_window_flags.cpp
    LINK_LIBS
        SDL3::SDL3
)

mu_add_test(
    NAME bundled_font_role_tests
    SOURCES
        test_bundled_font_roles.cpp
    LINK_LIBS MuClient
)

add_test(
    NAME bundled_font_assets
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_ASSET_SOURCE=${CMAKE_SOURCE_DIR}/src/bin"
        "-DMU_COPY_SCRIPT=${CMAKE_SOURCE_DIR}/cmake/CopyRuntimeAssets.cmake"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_bundled_font_assets.cmake"
)

add_test(
    NAME copy_runtime_assets
    COMMAND "${CMAKE_COMMAND}"
        -P "${CMAKE_SOURCE_DIR}/tests/test_copy_runtime_assets.cmake"
)
