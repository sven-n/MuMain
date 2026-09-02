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

add_test(
    NAME inventory_window_captures_hover
    COMMAND ${CMAKE_COMMAND}
        -DMU_INVENTORY_SOURCE=${CMAKE_SOURCE_DIR}/src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_inventory_mouse_capture.cmake
)

add_test(
    NAME npc_name_assets
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/test_npc_name_assets.py"
            "${CMAKE_SOURCE_DIR}"
)

mu_add_test(
    NAME npc_name_storage_tests
    SOURCES
        test_npc_name_storage.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Text/WideString.cpp
)

mu_add_test(
    NAME monster_health_bar_tests
    SOURCES
        test_monster_health_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/source/UI/Combat/MonsterHealthBar.cpp
)

mu_add_test(
    NAME ui_scaling_tests
    SOURCES
        test_ui_scaling.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS MuClient
)

add_test(
    NAME display_settings_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_CONFIG_CONSTANTS=${CMAKE_SOURCE_DIR}/src/source/Data/GameConfig/GameConfigConstants.h"
        "-DMU_CONFIG_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Data/GameConfig/GameConfig.cpp"
        "-DMU_WINMAIN_SOURCE=${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp"
        "-DMU_CONSOLE_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Core/Utilities/Log/muConsoleDebug.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_display_settings_contract.cmake"
)

mu_add_test(
    NAME character_name_lifecycle_tests
    SOURCES test_character_name_lifecycle.cpp
    LINK_LIBS MuClient
)
