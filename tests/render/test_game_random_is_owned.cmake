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

set(found "")
foreach(path IN LISTS sources)
    file(STRINGS "${path}" lines REGEX "rand")
    foreach(line IN LISTS lines)
        # The code of the line: no string or character literals, no comments.
        string(REGEX REPLACE "\"([^\"\\\\]|\\\\.)*\"" "\"\"" code "${line}")
        string(REGEX REPLACE "'([^'\\\\]|\\\\.)*'" "''" code "${code}")
        string(REGEX REPLACE "/\\*.*\\*/" "" code "${code}")
        string(REGEX REPLACE "//.*$" "" code "${code}")
        if(code MATCHES "^[ \t]*(\\*|/\\*|#[ \t]*include)")
            continue()
        endif()
        # rand or srand as a name of its own, or qualified by std:: or the global ::, however it is spelled.
        if(code MATCHES "(^|[^A-Za-z0-9_.>:])s?rand([^A-Za-z0-9_]|$)" OR
           code MATCHES "(^|[^A-Za-z0-9_])(std)?[ \t]*::[ \t]*s?rand([^A-Za-z0-9_]|$)")
            string(STRIP "${line}" line)
            list(APPEND found "${path}: ${line}")
        endif()
    endforeach()
endforeach()

if(found)
    list(JOIN found "\n" found)
    message(FATAL_ERROR "The CRT's random sequence is used; draw from Random::Rand / Random::SeedRand:\n${found}")
endif()
