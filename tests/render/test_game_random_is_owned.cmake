# The game draws from its own legacy sequence (Random::Rand, Random::SeedRand): its state can be read without drawing
# from it, which the draw purity check needs, and it gives the same values on every platform. A call to the CRT's
# rand() or srand() would start a second sequence nothing can see.
if(NOT DEFINED MU_SOURCE_DIRS)
    message(FATAL_ERROR "MU_SOURCE_DIRS is required")
endif()

set(sources "")
foreach(dir IN LISTS MU_SOURCE_DIRS)
    file(GLOB_RECURSE found_sources "${dir}/*.cpp" "${dir}/*.h" "${dir}/*.hpp")
    list(APPEND sources ${found_sources})
endforeach()
list(FILTER sources EXCLUDE REGEX "/(ThirdParty|Dotnet)/")

set(found "")
foreach(path IN LISTS sources)
    file(STRINGS "${path}" lines REGEX "(^|[^A-Za-z0-9_.>])(std::|::)?s?rand\\(")
    foreach(line IN LISTS lines)
        if(line MATCHES "^[ \t]*//")
            continue()
        endif()
        string(STRIP "${line}" line)
        list(APPEND found "${path}: ${line}")
    endforeach()
endforeach()

if(found)
    list(JOIN found "\n" found)
    message(FATAL_ERROR "The CRT's random sequence is used; draw from Random::Rand / Random::SeedRand:\n${found}")
endif()
