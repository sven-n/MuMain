foreach(required_variable IN ITEMS MU_LOGIN_SOURCE)
    if(NOT DEFINED ${required_variable})
        message(FATAL_ERROR "Missing required variable: ${required_variable}")
    endif()
endforeach()

file(READ "${MU_LOGIN_SOURCE}" login_source)

foreach(required_text IN ITEMS
        "Login send skipped; connection handle="
        "Login packet staged, handle="
        "Login packet staging failed, handle="
        "usernameBytes.Clear()"
        "passwordBytes.Clear()")
    string(FIND "${login_source}" "${required_text}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Missing login diagnostic: ${required_text}")
    endif()
endforeach()

string(FIND "${login_source}" "catch\n" empty_catch_position)
if(NOT empty_catch_position EQUAL -1)
    message(FATAL_ERROR "Login send must not swallow exceptions")
endif()
