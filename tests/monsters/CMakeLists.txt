# The monster recorder (monsters design, section 10): opens monster and NPC
# models through the game's code and records what changes, so tests can check
# what the game takes from the monster data. A binary of its own, because the
# recorder takes over the model slots and the audio backend; the loaders need
# the client globals of Winmain.cpp.
mu_add_test(
    NAME monster_model_recorder_tests
    SOURCES
        MonsterCreationScope.cpp
        MonsterModelRecorder.cpp
        test_monster_model_recorder.cpp
        test_monster_model_loader.cpp
        test_monster_slot_sounds.cpp
        test_monster_model_changes.cpp
        test_monster_attacks_play.cpp
        test_monster_attack_models.cpp
        test_monster_sound_events.cpp
        test_monster_sound_event_loads.cpp
        test_monster_creation_models.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(monster_model_recorder_tests PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)
