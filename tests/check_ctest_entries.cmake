cmake_minimum_required(VERSION 3.25)

# A test binary gets one CTest entry per test case. doctest_discover_tests registers an entry per name the binary
# lists, so a name it drops (an empty one, or one shaped like doctest's own output), cuts or joins makes the counts
# differ. An entry that runs no case fails by itself (MU_TEST_RAN_NO_CASE in tests/CMakeLists.txt), but a dropped name
# leaves no entry that could fail; this check compares the counts instead. A doctest::skip() condition is evaluated
# both when the discovery lists the cases at build time and when this check counts them, so one that reads the
# environment can make the counts differ too.
# Usage: cmake -DTEST_EXECUTABLE=<test binary> -DTEST_EXECUTOR=<emulator, may be empty>
#              -DDISCOVERY_INCLUDE_FILE=<file doctest_discover_tests added to TEST_INCLUDE_FILES>
#              -P check_ctest_entries.cmake

foreach(variable TEST_EXECUTABLE DISCOVERY_INCLUDE_FILE)
    if(NOT DEFINED ${variable})
        message(FATAL_ERROR "${variable} must be set")
    endif()
endforeach()

# The include file includes the file the discovery wrote the entries to, once the test binary is built.
file(READ "${DISCOVERY_INCLUDE_FILE}" include_script)
if(NOT include_script MATCHES "include\\(\"([^\"]+)\"\\)")
    message(FATAL_ERROR "${DISCOVERY_INCLUDE_FILE} includes no file of discovered entries")
endif()
set(ctest_file "${CMAKE_MATCH_1}")
if(NOT EXISTS "${ctest_file}")
    message(FATAL_ERROR "${ctest_file} does not exist: build the test binary, which writes it")
endif()

# The discovery writes one add_test( line per entry. Counting the matches keeps a name out of any CMake list.
file(READ "${ctest_file}" discovered)
string(REGEX MATCHALL "(^|\n)add_test\\(" entries "${discovered}")
list(LENGTH entries entry_count)

execute_process(COMMAND ${TEST_EXECUTOR} "${TEST_EXECUTABLE}" --count
    OUTPUT_VARIABLE output
    RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "${TEST_EXECUTABLE} --count failed (${result}):\n${output}")
endif()
if(NOT output MATCHES "unskipped test cases passing the current filters: ([0-9]+)")
    message(FATAL_ERROR "${TEST_EXECUTABLE} --count printed no count:\n${output}")
endif()
set(case_count ${CMAKE_MATCH_1})

if(NOT entry_count EQUAL case_count)
    message(FATAL_ERROR "${entry_count} CTest entries for ${case_count} test cases in ${TEST_EXECUTABLE}. Either "
        "CTest's discovery dropped, cut or joined a test case name (tests/check_test_case_names.py lists the names it "
        "changes), or a doctest::skip() condition, such as one that reads an environment variable, gave another "
        "answer when the binary was built than it gives now (decide such a case at run time, see "
        "docs/build/reference-tests.md).")
endif()
message(STATUS "${entry_count} CTest entries for ${case_count} test cases")
