cmake_minimum_required(VERSION 3.25)

# The configure runs dotnet's first-time setup for the build's DOTNET_CLI_HOME before the build starts its dotnet steps
# with that home in parallel: on Linux and macOS, two first runs at once can fail on NuGet's named mutex
# ("'NuGet-Migrations'. One or more system calls failed"). Every DOTNET_CLI_HOME in the file must be that home, and
# the setup must come first.

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
