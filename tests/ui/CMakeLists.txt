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
    NAME no_legacy_text_measurement
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}/src/source
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_no_legacy_text_measurement.cmake
)
