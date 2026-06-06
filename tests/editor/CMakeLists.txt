# Editor-specific tests only.
# Shared test harness setup (doctest include, wine setup, mu_test_main,
# and mu_add_test) lives in the top-level tests/CMakeLists.txt.

# Guard against the editor leaking into a non-editor build (issue #359).
# When ENABLE_EDITOR=OFF, nothing under MuEditor/ should be compiled in. The
# editor sources are added to the MuClient library (Main is just the
# entry-point executable that links it), so MuClient is the target to inspect
# -- Main's own SOURCES never list them.
if(NOT ENABLE_EDITOR)
    # file(GENERATE) resolves the SOURCES generator expression at generate
    # time and writes one absolute path per line.
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/muclient_sources.txt"
        CONTENT "$<JOIN:$<TARGET_PROPERTY:MuClient,SOURCES>,\n>")

    # Pure-CMake check: no Python or other interpreter to find, so the test
    # always runs wherever this project builds (CMake is always present),
    # instead of silently skipping when an interpreter is missing.
    # PROJECT_SOURCE_DIR (not CMAKE_SOURCE_DIR) so the editor path stays
    # correct even if MuMain is ever pulled in as a subdirectory of a larger
    # CMake project. Arguments are quoted to survive paths containing spaces.
    add_test(
        NAME editor_leak
        COMMAND ${CMAKE_COMMAND}
            "-DSOURCES_FILE=${CMAKE_CURRENT_BINARY_DIR}/muclient_sources.txt"
            "-DEDITOR_DIR=${PROJECT_SOURCE_DIR}/src/MuEditor"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/test_leak.cmake")
endif()
