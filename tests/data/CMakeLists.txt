mu_add_test(
    NAME test_item_database
    SOURCES
        test_item_database.cpp
        test_item_json.cpp
        test_item_model_json.cpp
        test_item_model_display_json.cpp
        test_item_model_glow_json.cpp
        test_item_model_problems.cpp
        test_localized_string.cpp
    LINK_LIBS
        MuClient
)

# Reads the item and item model files shipped in src/bin/Data through the real
# loader. The loader pulls in client globals defined in Winmain.cpp, so it is
# linked here (same as test_audio_playback).
mu_add_test(
    NAME test_item_database_files
    SOURCES
        test_item_database_files.cpp
        test_item_model_files.cpp
        test_item_rules.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(test_item_database_files PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)

# The effect catalogue (Data/Effects): the format, the compiled symbol lists
# and the shipped files. The registry check pulls in the effect code, which
# needs the client globals of Winmain.cpp. The effect browser test reads the
# switches of ZzzEffect.cpp (MU_TEST_SOURCE_DIR) and, in editor builds, checks
# the browser's model.
mu_add_test(
    NAME test_effect_types
    SOURCES
        test_effect_types.cpp
        test_effect_browser.cpp
        ${CMAKE_SOURCE_DIR}/tests/effects/EffectSourceCases.cpp
        ${CMAKE_SOURCE_DIR}/tests/effects/EffectTestData.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(test_effect_types PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
    MU_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}/src/source"
)
target_include_directories(test_effect_types PRIVATE ${CMAKE_SOURCE_DIR}/tests/effects)

# The effect registry before the effect catalogue is loaded: a binary of its
# own, so no other test has built the registry yet.
mu_add_test(
    NAME test_effect_registry_before_load
    SOURCES
        test_effect_registry_before_load.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
