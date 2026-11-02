# The game draws from its own legacy sequence (Random::Rand, Random::SeedRand): its state can be read without drawing
# from it, which the draw purity check needs, and it gives the same values on every platform. A use of the CRT's
# rand() or srand() would start a second sequence nothing can see.
if(NOT DEFINED MU_SOURCE_DIRS)
    message(FATAL_ERROR "MU_SOURCE_DIRS is required")
endif()

set(sources "")
foreach(dir IN LISTS MU_SOURCE_DIRS)
    file(GLOB_RECURSE found_sources "${dir}/*.c" "${dir}/*.cc" "${dir}/*.cpp" "${dir}/*.cxx" "${dir}/*.h" "${dir}/*.hpp"
         "${dir}/*.inl")
    list(APPEND sources ${found_sources})
endforeach()
list(FILTER sources EXCLUDE REGEX "/(ThirdParty|Dotnet)/")
# A guard that reads nothing passes nothing.
if(NOT sources)
    message(FATAL_ERROR "No sources under ${MU_SOURCE_DIRS}")
endif()

# What else is not code, whichever starts first, as a tokenizer reads it: a string, a character literal (a few
# characters, so a digit separator opens none), a line comment, a block comment, an include line.
set(not_code "\"[^\"\\\\\n]*(\\\\.[^\"\\\\\n]*)*\"")
string(APPEND not_code "|'(\\\\.|[^'\\\\\n])[^'\\\\\n]?[^'\\\\\n]?[^'\\\\\n]?'")
string(APPEND not_code "|//[^\n]*")
string(APPEND not_code "|/\\*[^*]*\\*+([^/*][^*]*\\*+)*/")
string(APPEND not_code "|#[ \t]*include[^\n]*")
# A raw string's opener; its delimiter is the first group.
set(raw_open "R\"([^ ()\\\\\t\n\"]*)\\(")

set(found "")
foreach(path IN LISTS sources)
    file(STRINGS "${path}" mentions REGEX "rand")
    if(NOT mentions)
        continue()
    endif()
    file(READ "${path}" text)
    # A raw string ends at its own delimiter, which a regular expression cannot match: where one may start, the file
    # is read token by token, the leftmost first, until no raw string is left; the rest goes in one replace.
    set(code "")
    while(text MATCHES "${raw_open}")
        if(NOT text MATCHES "${raw_open}|${not_code}")
            break()
        endif()
        set(token "${CMAKE_MATCH_0}")
        string(FIND "${text}" "${token}" begin)
        string(SUBSTRING "${text}" 0 ${begin} before)
        string(APPEND code "${before} ")
        string(LENGTH "${token}" length)
        math(EXPR after "${begin} + ${length}")
        string(SUBSTRING "${text}" ${after} -1 text)
        if(token MATCHES "^${raw_open}$")
            set(close ")${CMAKE_MATCH_1}\"")
            string(FIND "${text}" "${close}" end)
            # No end: not a raw string; what follows is read as code.
            if(NOT end EQUAL -1)
                string(LENGTH "${close}" length)
                math(EXPR after "${end} + ${length}")
                string(SUBSTRING "${text}" ${after} -1 text)
            endif()
        endif()
    endwhile()
    string(REGEX REPLACE "${not_code}" " " rest "${text}")
    string(APPEND code "${rest}")
    string(APPEND code "\n")
    # rand or srand as a name of its own, or qualified by std:: or the global ::, however it is spelled.
    string(REGEX MATCHALL "(^|[^A-Za-z0-9_.>:])s?rand[^A-Za-z0-9_]" bare "${code}")
    string(REGEX MATCHALL "(^|[^A-Za-z0-9_])(std)?[ \t]*::[ \t]*s?rand[^A-Za-z0-9_]" qualified "${code}")
    foreach(use IN LISTS bare qualified)
        string(STRIP "${use}" use)
        list(APPEND found "${path}: ${use}")
    endforeach()
endforeach()

if(found)
    list(JOIN found "\n" found)
    message(FATAL_ERROR "The CRT's random sequence is used; draw from Random::Rand / Random::SeedRand:\n${found}")
endif()
