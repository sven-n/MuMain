# The monster recorder (monsters design, section 10): opens monster and NPC
# models through the game's code and records what changes, so the old code and
# the monster data can be compared. A binary of its own, because the recorder
# takes over the model slots and the audio backend; the loaders need the client
# globals of Winmain.cpp.
mu_add_test(
    NAME monster_model_recorder_tests
    SOURCES
        MonsterModelRecorder.cpp
        test_monster_model_recorder.cpp
        test_monster_model_comparison.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(monster_model_recorder_tests PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)
