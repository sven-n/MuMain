cmake_minimum_required(VERSION 3.25)

# The configure runs dotnet's first-time setup for the build's DOTNET_CLI_HOME before the build starts its dotnet steps
# with that home in parallel: on Linux and macOS, two first runs at once can fail on NuGet's named mutex
# ("'NuGet-Migrations'. One or more system calls failed"). The setup must follow the line that defines that home, and
# every DOTNET_CLI_HOME in the file must be that home, the setup's first.

if(NOT DEFINED MU_CMAKE_SOURCE)
    message(FATAL_ERROR "MU_CMAKE_SOURCE is required")
endif()

file(READ "${MU_CMAKE_SOURCE}" cmake_source)
string(REGEX REPLACE "#[^\r\n]*" "" cmake_code "${cmake_source}")

set(home "\"DOTNET_CLI_HOME=\${DOTNET_TEMP_DIR_NATIVE}\"")
set(setup "${home} \"\${DOTNET_EXECUTABLE}\" help")
string(FIND "${cmake_code}" "${setup}" setup_at)
if(setup_at EQUAL -1)
    message(FATAL_ERROR
        "${MU_CMAKE_SOURCE} must run dotnet's first-time setup at configure: execute_process(COMMAND "
        "\${CMAKE_COMMAND} -E env ${setup}). `dotnet --version` and `--info` skip the setup.")
endif()
string(SUBSTRING "${cmake_code}" 0 ${setup_at} before_setup)
string(FIND "${before_setup}" "execute_process(" call_at REVERSE)
if(NOT call_at EQUAL -1)
    string(SUBSTRING "${before_setup}" ${call_at} -1 call_start)
endif()
if(call_at EQUAL -1 OR call_start MATCHES "\\)")
    message(FATAL_ERROR "The first-time setup in ${MU_CMAKE_SOURCE} must run in an execute_process, at configure.")
endif()

# Before the home's definition the setup would get an empty DOTNET_CLI_HOME and set up dotnet's default home, and
# inside an if() of its own it might not run at all.
set(definition "mu_native_path(\"\${DOTNET_TEMP_DIR}\" DOTNET_TEMP_DIR_NATIVE)")
string(FIND "${cmake_code}" "${definition}" definition_at)
if(definition_at EQUAL -1)
    message(FATAL_ERROR
        "${MU_CMAKE_SOURCE} no longer defines the build's dotnet home with ${definition}; update this check.")
endif()
string(LENGTH "${definition}" definition_length)
math(EXPR between_at "${definition_at} + ${definition_length}")
math(EXPR between_length "${call_at} - ${between_at}")
set(between "the setup comes first")
if(NOT between_length LESS 0)
    string(SUBSTRING "${cmake_code}" ${between_at} ${between_length} between)
endif()
if(NOT between MATCHES "^[ \t\r\n]*$")
    message(FATAL_ERROR
        "The first-time setup in ${MU_CMAKE_SOURCE} must follow ${definition} directly: before it the home is empty, "
        "and inside an if() of its own the setup might not run.")
endif()

# Every DOTNET_CLI_HOME names the same home, and the setup's comes first.
string(REGEX MATCHALL "[^ \t\r\n()]*DOTNET_CLI_HOME=[^ \t\r\n()]*" entries "${cmake_code}")
foreach(entry IN LISTS entries)
    if(NOT entry STREQUAL home)
        message(FATAL_ERROR
            "${MU_CMAKE_SOURCE} sets ${entry}, not ${home}: the configure's first-time setup does not cover that "
            "home.")
    endif()
endforeach()
string(FIND "${cmake_code}" "DOTNET_CLI_HOME=" first_at)
math(EXPR setup_home_at "${setup_at} + 1")
if(NOT first_at EQUAL setup_home_at)
    message(FATAL_ERROR "${MU_CMAKE_SOURCE} uses DOTNET_CLI_HOME before the configure's first-time setup runs.")
endif()
