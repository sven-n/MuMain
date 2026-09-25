mu_add_test(
    NAME test_item_database
    SOURCES
        test_item_database.cpp
        test_item_json.cpp
        test_localized_string.cpp
    LINK_LIBS
        MuClient
)

# Reads the item files shipped in src/bin/Data through the real loader. The
# loader pulls in client globals defined in Winmain.cpp, so it is linked here
# (same as test_audio_playback).
mu_add_test(
    NAME test_item_database_files
    SOURCES
        test_item_database_files.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(test_item_database_files PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)

# One-time check for phase 3 (item rules into data); see the file header.
mu_add_test(
    NAME test_item_rules_equivalence
    SOURCES
        test_item_rules_equivalence.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(test_item_rules_equivalence PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)
