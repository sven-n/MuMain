# The owned-random guard (test_game_random_is_owned.cmake) flags every compiled fixture that calls the CRT's rand() or
# srand(), however its source spells the call, and none that only mentions them or has a rand() of its own.
foreach(name MU_GUARD MU_SYMBOL_TOOL MU_USES MU_CLEAN MU_WORK)
    if(NOT DEFINED ${name})
        message(FATAL_ERROR "${name} is required")
    endif()
endforeach()

# One object's path, comparable however the list and the symbol tool spell it.
function(normalize path out)
    file(TO_CMAKE_PATH "${path}" path)
    get_filename_component(path "${path}" ABSOLUTE)
    if(CMAKE_HOST_WIN32)
        string(TOLOWER "${path}" path)
    endif()
    set(${out} "${path}" PARENT_SCOPE)
endfunction()

set(found "${MU_WORK}/crt_random_uses.txt.found")
file(REMOVE "${found}")
execute_process(COMMAND "${CMAKE_COMMAND}" "-DMU_OBJECT_LIST=${MU_USES}" "-DMU_SYMBOL_TOOL=${MU_SYMBOL_TOOL}"
                        "-DMU_OUTPUT=${MU_WORK}/crt_random_uses.txt" -P "${MU_GUARD}"
                RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE out)
# Findings come from this run only: a guard that could not read every object found nothing.
if(NOT EXISTS "${found}")
    message(FATAL_ERROR "The guard did not read the objects that call rand() or srand():\n${out}")
endif()
file(STRINGS "${MU_USES}" uses)
file(STRINGS "${found}" flagged)
set(flagged_paths "")
foreach(entry IN LISTS flagged)
    normalize("${entry}" entry)
    list(APPEND flagged_paths "${entry}")
endforeach()
set(missed "")
foreach(object IN LISTS uses)
    normalize("${object}" path)
    if(NOT path IN_LIST flagged_paths)
        list(APPEND missed "${object}")
    endif()
endforeach()
if(result EQUAL 0 OR missed)
    message(FATAL_ERROR "The guard let these CRT calls through: ${missed}\n${out}")
endif()

# The same objects, unreadable, after a run that found them all: the guard fails and keeps none of those findings.
set(missing "")
foreach(object IN LISTS uses)
    get_filename_component(name "${object}" NAME)
    list(APPEND missing "${MU_WORK}/crt_random_missing/${name}")
endforeach()
list(JOIN missing "\n" missing)
file(WRITE "${MU_WORK}/crt_random_missing.txt" "${missing}\n")
file(WRITE "${MU_WORK}/crt_random_missing.txt.found" "${missing}")
execute_process(COMMAND "${CMAKE_COMMAND}" "-DMU_OBJECT_LIST=${MU_WORK}/crt_random_missing.txt"
                        "-DMU_SYMBOL_TOOL=${MU_SYMBOL_TOOL}" "-DMU_OUTPUT=${MU_WORK}/crt_random_missing.txt"
                        -P "${MU_GUARD}"
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
if(result EQUAL 0 OR EXISTS "${MU_WORK}/crt_random_missing.txt.found")
    message(FATAL_ERROR "The guard kept findings for objects it could not read")
endif()

execute_process(COMMAND "${CMAKE_COMMAND}" "-DMU_OBJECT_LIST=${MU_CLEAN}" "-DMU_SYMBOL_TOOL=${MU_SYMBOL_TOOL}"
                        "-DMU_OUTPUT=${MU_WORK}/crt_random_clean.txt" -P "${MU_GUARD}"
                RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE out)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "The guard flagged code that calls neither rand() nor srand():\n${out}")
endif()
