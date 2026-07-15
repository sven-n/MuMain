cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED REPO_ROOT)
    message(FATAL_ERROR "REPO_ROOT must point to the repository root")
endif()

set(MU_CMAKE_FILE "${REPO_ROOT}/src/CMakeLists.txt")
file(READ "${MU_CMAKE_FILE}" mu_cmake)
string(CONCAT obsolete_sdl3_gate "MU_ENABLE_" "SDL3")

string(FIND "${mu_cmake}" "${obsolete_sdl3_gate}" obsolete_cmake_gate)
if(NOT obsolete_cmake_gate EQUAL -1)
    message(FATAL_ERROR "${obsolete_sdl3_gate} must not appear in src/CMakeLists.txt")
endif()

string(FIND "${mu_cmake}" "add_subdirectory(ThirdParty/SDL EXCLUDE_FROM_ALL)" sdl_subdirectory)
if(sdl_subdirectory EQUAL -1)
    message(FATAL_ERROR "MuClient must build the vendored SDL3 dependency")
endif()

string(REGEX MATCH "target_link_libraries\\(MuClient PUBLIC[ \t\r\n]+SDL3::SDL3" sdl_link "${mu_cmake}")
if(NOT sdl_link)
    message(FATAL_ERROR "MuClient must link SDL3::SDL3 publicly")
endif()

string(FIND "${mu_cmake}" "MU_HAS_DOTNET_AOT" dotnet_aot_definition)
if(dotnet_aot_definition EQUAL -1)
    message(FATAL_ERROR "Native AOT builds must define MU_HAS_DOTNET_AOT")
endif()

file(GLOB_RECURSE production_sources
    "${REPO_ROOT}/src/source/*.cpp"
    "${REPO_ROOT}/src/source/*.h"
)
foreach(source_file IN LISTS production_sources)
    file(READ "${source_file}" source_content)
    string(FIND "${source_content}" "${obsolete_sdl3_gate}" obsolete_source_gate)
    if(NOT obsolete_source_gate EQUAL -1)
        file(RELATIVE_PATH relative_source "${REPO_ROOT}" "${source_file}")
        message(FATAL_ERROR "Obsolete ${obsolete_sdl3_gate} gate remains in ${relative_source}")
    endif()
endforeach()

file(GLOB_RECURSE test_sources
    "${REPO_ROOT}/tests/*.cmake"
    "${REPO_ROOT}/tests/*.cpp"
    "${REPO_ROOT}/tests/*.h"
    "${REPO_ROOT}/tests/CMakeLists.txt"
)
foreach(test_file IN LISTS test_sources)
    file(READ "${test_file}" test_content)
    string(FIND "${test_content}" "${obsolete_sdl3_gate}" obsolete_test_gate)
    if(NOT obsolete_test_gate EQUAL -1)
        file(RELATIVE_PATH relative_test "${REPO_ROOT}" "${test_file}")
        message(FATAL_ERROR "Obsolete ${obsolete_sdl3_gate} test remains in ${relative_test}")
    endif()
endforeach()

set(SIGNAL_HANDLER_FILE "${REPO_ROOT}/src/source/Core/Platform/posix/PosixSignalHandlers.cpp")
file(READ "${SIGNAL_HANDLER_FILE}" signal_handler_source)
string(FIND "${signal_handler_source}" "MU_HAS_DOTNET_AOT" dotnet_aot_gate)
if(dotnet_aot_gate EQUAL -1)
    message(FATAL_ERROR "POSIX SIGSEGV ownership must use MU_HAS_DOTNET_AOT")
endif()
