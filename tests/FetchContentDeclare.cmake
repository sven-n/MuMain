# mu_read_fetchcontent_declare(<root_cmake> <name> <code_variable> <declare_variable> <declare_at_variable>)
#
# Sets <code_variable> to <root_cmake> without its comments, <declare_variable> to the one FetchContent_Declare(<name>
# ...) in it and <declare_at_variable> to where that starts in the code; stops when there is not exactly one. A '#' or
# ')' inside a quoted argument stays part of it, so neither cuts the code or the declare short.
function(mu_read_fetchcontent_declare root_cmake name code_variable declare_variable declare_at_variable)
    file(READ "${root_cmake}" source)
    set(quoted_argument "\"([^\"\\\\]|\\\\.)*\"")
    # A quoted argument is matched as a whole and kept; a comment outside one is matched and dropped.
    string(REGEX REPLACE "(${quoted_argument})|#[^\r\n]*" "\\1" code "${source}")
    string(REGEX MATCHALL "FetchContent_Declare\\([ \t\r\n]*${name}[ \t\r\n]([^)\"]|${quoted_argument})*\\)"
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
