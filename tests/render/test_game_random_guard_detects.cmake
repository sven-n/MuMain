# The owned-random guard (test_game_random_is_owned.cmake) flags every compiled fixture that calls the CRT's rand() or
# srand(), however its source spells the call, and none that only mentions them or has a rand() of its own.
foreach(name MU_GUARD MU_SYMBOL_TOOL MU_USES MU_CLEAN MU_WORK)
    if(NOT DEFINED ${name})
        message(FATAL_ERROR "${name} is required")
    endif()
endforeach()

execute_process(COMMAND "${CMAKE_COMMAND}" "-DMU_OBJECT_LIST=${MU_USES}" "-DMU_SYMBOL_TOOL=${MU_SYMBOL_TOOL}"
                        "-DMU_OUTPUT=${MU_WORK}/crt_random_uses.txt" -P "${MU_GUARD}"
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
file(STRINGS "${MU_USES}" uses)
file(STRINGS "${MU_WORK}/crt_random_uses.txt.found" flagged)
set(missed "")
foreach(object IN LISTS uses)
    get_filename_component(name "${object}" NAME)
    set(seen FALSE)
    foreach(entry IN LISTS flagged)
        file(TO_CMAKE_PATH "${entry}" entry)
        get_filename_component(flagged_name "${entry}" NAME)
        if(flagged_name STREQUAL name)
            set(seen TRUE)
        endif()
    endforeach()
    if(NOT seen)
        list(APPEND missed "${name}")
    endif()
endforeach()
if(result EQUAL 0 OR missed)
    message(FATAL_ERROR "The guard let these CRT calls through: ${missed}")
endif()

execute_process(COMMAND "${CMAKE_COMMAND}" "-DMU_OBJECT_LIST=${MU_CLEAN}" "-DMU_SYMBOL_TOOL=${MU_SYMBOL_TOOL}"
                        "-DMU_OUTPUT=${MU_WORK}/crt_random_clean.txt" -P "${MU_GUARD}"
                RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE out)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "The guard flagged code that calls neither rand() nor srand():\n${out}")
endif()
