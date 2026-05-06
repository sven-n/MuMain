 # Editor-specific tests only.
 # Shared test harness setup (doctest include, wine setup, mu_test_main,
 # and mu_add_test) must remain in the top-level tests/CMakeLists.txt.

# Add Editor Leak Test (only runs when ENABLE_EDITOR is OFF)
if(NOT ENABLE_EDITOR)
    # Generate a text file containing all sources of the Main target
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/main_sources.txt"
        CONTENT "$<JOIN:$<TARGET_PROPERTY:Main,SOURCES>,\n>")

    # Add a test that runs a Python script to verify MuEditor isn't in those sources
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
         add_test(NAME test_editor_leak
                   COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/editor/test_leak.py "${CMAKE_CURRENT_BINARY_DIR}/main_sources.txt")
    else()
         message(STATUS "Skipping test_editor_leak: Python3 interpreter not found")
    endif()
endif()
