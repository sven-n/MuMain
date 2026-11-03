# The game draws from its own legacy sequence (Random::Rand, Random::SeedRand): its state can be read without drawing
# from it, which the draw purity check needs, and it gives the same values on every platform. A call to the CRT's
# rand() or srand() would start a second sequence nothing can see.
#
# The compiled objects say which functions the code calls, whatever the source spelling: macros, aliases, line
# continuations and literals are already resolved, and the game's own functions named rand are told apart from the
# CRT's. MU_OBJECT_LIST names the objects, one per line; MU_SYMBOL_TOOL is MSVC's link.exe or nm; MU_OUTPUT is a
# scratch file, and the objects that call the CRT are written to MU_OUTPUT.found.
foreach(name MU_OBJECT_LIST MU_SYMBOL_TOOL MU_OUTPUT)
    if(NOT DEFINED ${name})
        message(FATAL_ERROR "${name} is required")
    endif()
endforeach()

# MU_OUTPUT.found holds this run's findings only, and only once every object was read.
file(REMOVE "${MU_OUTPUT}.found")

file(STRINGS "${MU_OBJECT_LIST}" objects)
# Compiled code only: a target's objects can include resources (resource.rc.res), which hold none.
list(FILTER objects INCLUDE REGEX "\\.(obj|o)$")
# A guard that reads nothing passes nothing.
if(NOT objects)
    message(FATAL_ERROR "No objects listed in ${MU_OBJECT_LIST}")
endif()

get_filename_component(tool "${MU_SYMBOL_TOOL}" NAME_WE)
string(TOLOWER "${tool}" tool)
if(tool STREQUAL "link")
    # Every object's symbol table after its "Dump of file" line; the CRT functions are imported (__imp_rand) or not.
    list(TRANSFORM objects REPLACE "^(.*)$" "\"\\1\"" OUTPUT_VARIABLE quoted)
    list(JOIN quoted "\n" quoted)
    file(WRITE "${MU_OUTPUT}.rsp" "${quoted}\n")
    # The tables run to hundreds of megabytes: only the object headers and the lines that name a rand go on.
    file(WRITE "${MU_OUTPUT}.filter" "Dump of file\nrand\n")
    execute_process(COMMAND "${MU_SYMBOL_TOOL}" /dump /nologo /symbols "@${MU_OUTPUT}.rsp"
                    COMMAND findstr /L "/G:${MU_OUTPUT}.filter"
                    OUTPUT_FILE "${MU_OUTPUT}" RESULTS_VARIABLE results)
    set(object_line "^Dump of file (.+)$")
    set(use_line "UNDEF .* External +\\| (__imp_)?s?rand[ \t\r]*$")
else()
    # Each line starts with its object; the CRT functions are undefined, strong (U) or weak (w, v), with a leading _ on
    # Apple platforms. Defined symbols (T, W, V) are the code's own.
    execute_process(COMMAND "${MU_SYMBOL_TOOL}" -A ${objects} COMMAND grep rand OUTPUT_FILE "${MU_OUTPUT}"
                    RESULTS_VARIABLE results)
    set(object_line "")
    set(use_line "^(.+): *[Uvw] _?s?rand(@[^ ]*)?[ \t\r]*$")
endif()
# The symbol tool must succeed; the filter ran when it found lines (0) or none (1).
list(GET results 0 result)
list(GET results 1 filtered)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "${MU_SYMBOL_TOOL} could not read the objects (${result})")
endif()
if(NOT filtered MATCHES "^[01]$")
    message(FATAL_ERROR "The symbol filter did not run (${filtered})")
endif()

if(object_line)
    file(STRINGS "${MU_OUTPUT}" lines REGEX "${object_line}|${use_line}")
else()
    file(STRINGS "${MU_OUTPUT}" lines REGEX "${use_line}")
endif()
file(REMOVE "${MU_OUTPUT}")

set(found "")
set(object "")
foreach(line IN LISTS lines)
    if(object_line AND line MATCHES "${object_line}")
        set(object "${CMAKE_MATCH_1}")
    elseif(line MATCHES "${use_line}")
        if(NOT object_line)
            set(object "${CMAKE_MATCH_1}")
        endif()
        string(STRIP "${object}" object)
        list(APPEND found "${object}")
    endif()
endforeach()
list(REMOVE_DUPLICATES found)
list(JOIN found "\n" listed)

list(LENGTH objects count)
# Every object read: a filter that dropped the headers would otherwise pass anything.
if(object_line)
    list(FILTER lines INCLUDE REGEX "${object_line}")
    list(LENGTH lines read)
    if(NOT read EQUAL count)
        message(FATAL_ERROR "Read ${read} of ${count} objects")
    endif()
endif()
file(WRITE "${MU_OUTPUT}.found" "${listed}")
if(found)
    message(FATAL_ERROR "The CRT's random sequence is used; draw from Random::Rand / Random::SeedRand:\n${listed}")
endif()
message(STATUS "${count} objects call neither rand() nor srand()")
