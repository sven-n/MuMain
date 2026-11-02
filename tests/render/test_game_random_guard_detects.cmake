# The owned-random guard (test_game_random_is_owned.cmake) fails on every spelling of a CRT random call and passes
# code that only mentions one in a comment or a string.
foreach(name MU_GUARD MU_FIXTURES)
    if(NOT DEFINED ${name})
        message(FATAL_ERROR "${name} is required")
    endif()
endforeach()

set(missed "")
foreach(case bare spaced parenthesized c_file seed global_seed between_comments deref comment_in_string
             raw_delimiter raw_in_line_comments raw_in_block_comments)
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DMU_SOURCE_DIRS=${MU_FIXTURES}/${case}" -P "${MU_GUARD}"
                    RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
    if(result EQUAL 0)
        list(APPEND missed "${case}")
    endif()
endforeach()
if(missed)
    message(FATAL_ERROR "The guard let these CRT calls through: ${missed}")
endif()

execute_process(COMMAND "${CMAKE_COMMAND}" "-DMU_SOURCE_DIRS=${MU_FIXTURES}/clean" -P "${MU_GUARD}"
                RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE out)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "The guard flagged code that only mentions rand():
${out}")
endif()
