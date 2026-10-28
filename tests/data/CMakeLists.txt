mu_add_test(
    NAME test_item_database
    SOURCES
        test_item_check_targets.cpp
        test_item_code_references.cpp
        test_item_database.cpp
        test_item_description_placeholders.cpp
        test_item_edit_history.cpp
        test_item_json.cpp
        test_item_list_model.cpp
        test_item_model_json.cpp
        test_item_model_display_json.cpp
        test_item_model_glow_json.cpp
        test_item_model_material_json.cpp
        test_item_model_problems.cpp
        test_item_model_tool.cpp
        test_item_names.cpp
        test_item_stat_file_change.cpp
        test_item_stats_json.cpp
        test_item_users.cpp
        test_localized_string.cpp
    LINK_LIBS
        MuClient
)

# The light mood of the world maps (Data/Maps/LightMood.json).
mu_add_test(
    NAME light_mood_data_tests
    SOURCES
        test_light_mood_json.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(light_mood_data_tests PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)

# The server groups of the login screen (Data/Local/ServerList.json). The
# window test draws the editor's Server List window, which needs the client
# globals of Winmain.cpp.
mu_add_test(
    NAME server_list_data_tests
    SOURCES
        test_server_list_edits.cpp
        test_server_list_json.cpp
        test_server_list_manager.cpp
        test_server_list_window.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(server_list_data_tests PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)

# The monster list (Data/Monsters/Monsters.json), its table, the format of
# the original client's name files and the editor's monster tools. The table
# needs the client globals of Winmain.cpp.
mu_add_test(
    NAME monster_data_tests
    SOURCES
        test_legacy_monster_kind.cpp
        test_monster_attack_checks.cpp
        test_monster_attack_readers.cpp
        test_monster_attacks.cpp
        test_monster_browser_window.cpp
        test_monster_check_targets.cpp
        test_monster_checks_window.cpp
        test_monster_code_references.cpp
        test_monster_data_edits.cpp
        test_monster_data_files.cpp
        test_monster_data_users.cpp
        test_monster_database.cpp
        test_monster_edit_history.cpp
        test_monster_json.cpp
        test_monster_list_model.cpp
        test_monster_model_files.cpp
        test_monster_model_values.cpp
        test_monster_models_json.cpp
        test_monster_names.cpp
        test_monster_npc_name_files.cpp
        test_monster_tool_windows.cpp
        test_monster_translations_window.cpp
        test_names_grid.cpp
        test_npc_name_text.cpp
        test_transformation_ring.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(monster_data_tests PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)

# The look profile of the Enhanced rendering features (Data/Render/Look.json) and the player's own
# tuning of it ([EnhancedTuning] of config.ini).
mu_add_test(
    NAME look_profile_data_tests
    SOURCES
        test_look_profile_json.cpp
        test_personal_tuning.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(look_profile_data_tests PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)

# Reads the item and item model files shipped in src/bin/Data through the real
# loader. The loader pulls in client globals defined in Winmain.cpp, so it is
# linked here (same as test_audio_playback).
mu_add_test(
    NAME test_item_database_files
    SOURCES
        test_helper_pickup_filters.cpp
        test_dark_lord_pet_tooltip.cpp
        test_item_database_files.cpp
        test_item_descriptions.cpp
        test_item_exchange_file.cpp
        test_item_model_files.cpp
        test_item_requirement_weights.cpp
        test_item_rules.cpp
        test_item_stats.cpp
        test_item_tooltip_layouts.cpp
        test_item_sync_diff.cpp
        test_item_tooltip_text.cpp
        test_item_tooltips.cpp
        test_item_value_formulas.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
        # The test's version 14 model is encrypted with Crypto++'s ciphers.
        MuCryptopp
)
target_compile_definitions(test_item_database_files PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
    MU_TEST_TOOLTIP_SNAPSHOT="${CMAKE_CURRENT_SOURCE_DIR}/item_tooltip_snapshot.txt"
    MU_TEST_CHAIN_TEXTS_BEFORE="${CMAKE_CURRENT_SOURCE_DIR}/item_chain_texts_before.json"
)

# Animates the shipped player model the way the character pre-pass does.
mu_add_test(
    NAME test_character_bones
    SOURCES
        test_character_bones.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(test_character_bones PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)

# The effect catalogue (Data/Effects): the format, the compiled symbol lists
# and the shipped files. The registry check pulls in the effect code, which
# needs the client globals of Winmain.cpp. The effect browser test reads the
# switches of ZzzEffect.cpp and the conditions of MoveEffect's shared code
# (MU_TEST_SOURCE_DIR) and, in editor builds, checks the browser's model and
# the parts of its previews that need no frame.
mu_add_test(
    NAME test_effect_types
    SOURCES
        test_effect_types.cpp
        test_effect_browser.cpp
        test_effect_preview.cpp
        test_effect_world_preview.cpp
        test_effect_call_sites.cpp
        ${CMAKE_SOURCE_DIR}/tests/effects/EffectSourceCases.cpp
        ${CMAKE_SOURCE_DIR}/tests/effects/EffectSourceCondition.cpp
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

# The model files that replace a model's BMD (Data/Models/ModelFiles.json) and
# the developer overlay (MU_DATA_OVERLAY). The table the game reads at start
# (ReadModelFileTable) comes with the client globals of Winmain.cpp.
mu_add_test(
    NAME model_files_data_tests
    SOURCES
        test_model_files.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(model_files_data_tests PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
    MU_TEST_ENUM_HEADER="${CMAKE_SOURCE_DIR}/src/source/Core/Globals/_enum.h"
)
