add_test(
    NAME mandatory_sdl3_contract
    COMMAND ${CMAKE_COMMAND}
        -DREPO_ROOT=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_mandatory_sdl3.cmake
)
add_test(
  NAME build-clientlibrary-config-isolation
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test_clientlibrary_config_isolation.cmake
)
