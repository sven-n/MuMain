# Test tree. New test modules go in their own subdirectory and call
# add_subdirectory() below. Keep the entry point here minimal — module
# CMakeLists.txt files own their own sources, link, and call mu_add_test().

include(third_party/doctest/doctest.cmake)

# When cross-compiling Windows binaries on a non-Windows host (CI's Linux
# runner, WSL/MinGW), wine has to launch the .exe. Setting this once makes
# both add_test and doctest_discover_tests pick it up via the
# CROSSCOMPILING_EMULATOR target property.
if(WIN32 AND NOT CMAKE_HOST_WIN32 AND NOT CMAKE_CROSSCOMPILING_EMULATOR)
    find_program(WINE_EXECUTABLE wine)
    if(NOT WINE_EXECUTABLE)
        message(FATAL_ERROR
            "BUILD_TESTING=ON on a non-Windows host requires wine to run "
            "the cross-compiled test binaries. Install it (Ubuntu/Debian: "
            "sudo apt-get install wine wine32) or configure with "
            "-DBUILD_TESTING=OFF.")
    endif()
    set(CMAKE_CROSSCOMPILING_EMULATOR ${WINE_EXECUTABLE} CACHE STRING "" FORCE)
endif()

add_library(mu_test_main STATIC main.cpp)
target_include_directories(mu_test_main PUBLIC third_party/doctest)
target_compile_features(mu_test_main PUBLIC cxx_std_20)
# doctest declares std::tuple and other standard templates itself unless told to include their headers, and MSVC
# warns that a program may not declare them (C5285).
target_compile_definitions(mu_test_main PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
# Test sources contain non-ASCII wide string literals. MSVC defaults to the
# system codepage when reading sources, which mangles UTF-8 characters into
# wrong codepoints and breaks every non-Latin language test. /utf-8 forces
# both the source charset and the narrow execution charset to UTF-8; the wide
# execution charset stays UTF-16 (Windows wchar_t).
if(MSVC)
    target_compile_options(mu_test_main PUBLIC /utf-8)
endif()

# Longest one TEST_CASE may run before ctest stops it and reports it as
# failed. The slowest case takes about a second; without a limit, a case that
# hangs holds its CI job until the runner gives up hours later.
set(MU_TEST_CASE_TIMEOUT_SECONDS 120)

# What doctest prints when an entry's filter matches no case: it runs nothing and still exits 0. Such an entry fails,
# so a name the discovery cut or joined (see tests/check_test_case_names.py) cannot pass unseen. The '.' stands for
# doctest's '|': on Windows the discovery's post-build command line would read a '|' as a pipe.
set(MU_TEST_RAN_NO_CASE "test cases: +0 . +0 passed")
# A dropped name leaves no entry that could fail: every test binary also gets an entry that compares its CTest
# entries with its test cases.
set(MU_CHECK_CTEST_ENTRIES "${CMAKE_CURRENT_LIST_DIR}/check_ctest_entries.cmake")

# Helper for module CMakeLists. Usage:
#   mu_add_test(NAME <test_name> SOURCES a.cpp b.cpp LINK_LIBS lib1 lib2 [RESOURCE_LOCK <lock>])
# Registers each TEST_CASE inside the binary as its own CTest entry, so
# failures point at the specific case in CI logs.
set(MU_TEST_WINMAIN "${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp")
# Most tests need the client globals of Winmain.cpp; compile it once for all of them, not per test.
add_library(mu_test_winmain OBJECT "${MU_TEST_WINMAIN}")
target_link_libraries(mu_test_winmain PUBLIC MuClient)
target_include_directories(mu_test_winmain PRIVATE ${CMAKE_SOURCE_DIR}/src/source)
if(MSVC)
    target_compile_options(mu_test_winmain PRIVATE /utf-8)
endif()

function(mu_add_test)
    cmake_parse_arguments(MAT "" "NAME;RESOURCE_LOCK" "SOURCES;LINK_LIBS" ${ARGN})
    list(FIND MAT_SOURCES "${MU_TEST_WINMAIN}" winmain_index)
    if(NOT winmain_index EQUAL -1)
        list(REMOVE_AT MAT_SOURCES ${winmain_index})
        list(APPEND MAT_LINK_LIBS mu_test_winmain)
    endif()
    add_executable(${MAT_NAME} ${MAT_SOURCES})
    target_link_libraries(${MAT_NAME} PRIVATE mu_test_main ${MAT_LINK_LIBS})
    target_include_directories(${MAT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/source
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest
    )
    if(MSVC)
        target_compile_options(${MAT_NAME} PRIVATE /utf-8)
    endif()
    # Cases of one binary share fixed temp paths, so `ctest --parallel` runs them one at a time; binaries overlap,
    # except binaries that name the same RESOURCE_LOCK because they write the same file.
    if(NOT MAT_RESOURCE_LOCK)
        set(MAT_RESOURCE_LOCK ${MAT_NAME})
    endif()
    doctest_discover_tests(${MAT_NAME} PROPERTIES
        TIMEOUT ${MU_TEST_CASE_TIMEOUT_SECONDS}
        RESOURCE_LOCK ${MAT_RESOURCE_LOCK}
        FAIL_REGULAR_EXPRESSION "${MU_TEST_RAN_NO_CASE}")
    # The file doctest_discover_tests writes the entries to: doctest.cmake names it after a hash of the TEST_SPEC and
    # EXTRA_ARGS, both empty here.
    string(SHA1 discovery_hash " ")
    string(SUBSTRING ${discovery_hash} 0 7 discovery_hash)
    get_property(emulator TARGET ${MAT_NAME} PROPERTY CROSSCOMPILING_EMULATOR)
    add_test(NAME ${MAT_NAME}_has_one_ctest_entry_per_case
        COMMAND "${CMAKE_COMMAND}"
            "-DTEST_EXECUTABLE=$<TARGET_FILE:${MAT_NAME}>"
            "-DTEST_EXECUTOR=${emulator}"
            "-DCTEST_FILE=${CMAKE_CURRENT_BINARY_DIR}/${MAT_NAME}_tests-${discovery_hash}.cmake"
            -P "${MU_CHECK_CTEST_ENTRIES}")
    set_tests_properties(${MAT_NAME}_has_one_ctest_entry_per_case PROPERTIES
        TIMEOUT ${MU_TEST_CASE_TIMEOUT_SECONDS}
        RESOURCE_LOCK ${MAT_RESOURCE_LOCK})
endfunction()

find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_subdirectory(static)
add_subdirectory(benchmark)

add_subdirectory(text)
add_subdirectory(effects)

add_subdirectory(monsters)

add_subdirectory(ui)

add_subdirectory(editor)

add_subdirectory(render)

add_subdirectory(network)

add_subdirectory(platform)

add_subdirectory(core)

add_subdirectory(data)

add_subdirectory(gamelogic)
