add_test(
    NAME mandatory_sdl3_contract
    COMMAND ${CMAKE_COMMAND}
        -DREPO_ROOT=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_mandatory_sdl3.cmake
)
