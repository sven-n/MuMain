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

# Every other DOTNET_CLI_HOME comes after the setup and names the same home.
set(rest "${cmake_code}")
set(offset 0)
while(TRUE)
    string(FIND "${rest}" "DOTNET_CLI_HOME=" found_at)
    if(found_at EQUAL -1)
        break()
    endif()
    math(EXPR at "${offset} + ${found_at}")
    math(EXPR quoted_at "${at} - 1")
    string(LENGTH "${home}" home_length)
    string(SUBSTRING "${cmake_code}" ${quoted_at} ${home_length} entry)
    if(NOT entry STREQUAL home)
        message(FATAL_ERROR
            "${MU_CMAKE_SOURCE} sets a DOTNET_CLI_HOME other than \${DOTNET_TEMP_DIR_NATIVE}: the configure's "
            "first-time setup does not cover it.")
    endif()
    if(at LESS setup_at)
        message(FATAL_ERROR
            "${MU_CMAKE_SOURCE} uses DOTNET_CLI_HOME before the configure's first-time setup runs.")
    endif()
    math(EXPR next "${found_at} + 1")
    string(SUBSTRING "${rest}" ${next} -1 rest)
    math(EXPR offset "${offset} + ${next}")
endwhile()
