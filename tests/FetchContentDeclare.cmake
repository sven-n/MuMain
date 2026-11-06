# A quoted CMake argument. A '#' or ')' inside one stays part of it, so neither cuts the code or a declare short.
set(MU_CMAKE_QUOTED_ARGUMENT "\"([^\"\\\\]|\\\\.)*\"")

# mu_read_cmake_code(<cmake_file> <code_variable>)
#
# Sets <code_variable> to <cmake_file> without its comments.
function(mu_read_cmake_code cmake_file code_variable)
    file(READ "${cmake_file}" source)
    # A line's code before its comment: text without '#' or a whole quoted argument, which may hold one. Both groups
    # the replacement keeps take part in every match, as CMake before 3.29 requires.
    set(code_before_comment "(([^\"#\r\n]|${MU_CMAKE_QUOTED_ARGUMENT})*)")
    string(REGEX REPLACE "(^|\n)${code_before_comment}#[^\r\n]*" "\\1\\2" code "${source}")
    set(${code_variable} "${code}" PARENT_SCOPE)
endfunction()

# mu_read_fetchcontent_declare_names(<root_cmake> <names_variable>)
#
# Sets <names_variable> to the names of the FetchContent_Declare calls in <root_cmake>'s code, in their order.
function(mu_read_fetchcontent_declare_names root_cmake names_variable)
    mu_read_cmake_code("${root_cmake}" code)
    string(REGEX MATCHALL "FetchContent_Declare\\([ \t\r\n]*[^ \t\r\n)\"]+" calls "${code}")
    set(names "")
    foreach(call IN LISTS calls)
        string(REGEX REPLACE "^FetchContent_Declare\\([ \t\r\n]*" "" name "${call}")
        list(APPEND names "${name}")
    endforeach()
    set(${names_variable} "${names}" PARENT_SCOPE)
endfunction()

# mu_read_fetchcontent_declare(<root_cmake> <name> <code_variable> <declare_variable> <declare_at_variable>)
#
# Sets <code_variable> to <root_cmake> without its comments, <declare_variable> to the one FetchContent_Declare(<name>
# ...) in it and <declare_at_variable> to where that starts in the code; stops when there is not exactly one.
function(mu_read_fetchcontent_declare root_cmake name code_variable declare_variable declare_at_variable)
    mu_read_cmake_code("${root_cmake}" code)
    string(REGEX MATCHALL "FetchContent_Declare\\([ \t\r\n]*${name}[ \t\r\n]([^)\"]|${MU_CMAKE_QUOTED_ARGUMENT})*\\)"
        declares "${code}")
    list(LENGTH declares declare_count)
    if(NOT declare_count EQUAL 1)
        message(FATAL_ERROR "Expected one FetchContent_Declare(${name} ...) in ${root_cmake}, found ${declare_count}")
    endif()
    string(FIND "${code}" "${declares}" declare_at)
    set(${code_variable} "${code}" PARENT_SCOPE)
    set(${declare_variable} "${declares}" PARENT_SCOPE)
    set(${declare_at_variable} "${declare_at}" PARENT_SCOPE)
endfunction()
