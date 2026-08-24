mu_add_test(
    NAME input_box_config_tests
    SOURCES test_input_box_config.cpp
    LINK_LIBS MuClient
)

mu_add_test(
    NAME text_search_tests
    SOURCES
        test_text_search.cpp
        ${CMAKE_SOURCE_DIR}/src/source/UI/Legacy/TextSearch.cpp
)

add_test(
    NAME chat_command_window_focus_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_chat_command_window_focus.cmake
)

add_test(
    NAME inventory_and_system_backdrops_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_inventory_and_system_backdrops.cmake
)

add_test(
    NAME exit_game_shutdown_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_exit_game_shutdown.cmake
)
