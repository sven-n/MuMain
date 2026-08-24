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
