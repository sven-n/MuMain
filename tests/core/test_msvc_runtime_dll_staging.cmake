if(NOT DEFINED MU_CMAKE_SOURCE)
    message(FATAL_ERROR "MU_CMAKE_SOURCE is required")
endif()

file(READ "${MU_CMAKE_SOURCE}" cmake_source)
string(REGEX REPLACE "#[^\r\n]*" "" cmake_code "${cmake_source}")

function(require_match source pattern description)
    string(REGEX MATCH "${pattern}" match "${source}")
    if(NOT match)
        message(FATAL_ERROR "Missing MSVC runtime DLL staging contract: ${description}")
    endif()
endfunction()

function(reject_match source pattern description)
    string(REGEX MATCH "${pattern}" match "${source}")
    if(match)
        message(FATAL_ERROR "Forbidden MSVC runtime DLL staging contract: ${description}")
    endif()
endfunction()

string(REGEX MATCHALL "\\$<TARGET_RUNTIME_DLLS:Main>" runtime_dll_occurrences "${cmake_code}")
list(LENGTH runtime_dll_occurrences runtime_dll_count)
if(NOT runtime_dll_count EQUAL 1)
    message(FATAL_ERROR
        "Expected exactly one $<TARGET_RUNTIME_DLLS:Main> occurrence, found ${runtime_dll_count}")
endif()

foreach(forbidden IN ITEMS DLL_SEARCH_PATH REQUIRED_DLL_NAMES REQUIRED_DLLS glew32)
    reject_match("${cmake_code}" "${forbidden}" "manual ${forbidden} staging")
endforeach()

string(FIND "${cmake_code}" "$<TARGET_RUNTIME_DLLS:Main>" staging_anchor)

string(SUBSTRING "${cmake_code}" 0 ${staging_anchor} before_staging)
string(FIND "${before_staging}" "add_custom_command(TARGET Main POST_BUILD" command_start REVERSE)
if(command_start EQUAL -1)
    message(FATAL_ERROR "Missing Main POST_BUILD staging command")
endif()
string(SUBSTRING "${cmake_code}" 0 ${command_start} before_staging_command)
string(FIND "${before_staging_command}" "if (MSVC)" block_start REVERSE)
if(block_start EQUAL -1)
    message(FATAL_ERROR "Missing dedicated flat if(MSVC) staging block")
endif()
math(EXPR block_header_length "${command_start} - ${block_start}")
string(SUBSTRING "${cmake_code}" ${block_start} ${block_header_length} block_header)
if(NOT block_header MATCHES "^if[ \t]*\\([ \t]*MSVC[ \t]*\\)[ \t\r\n]*$")
    message(FATAL_ERROR
        "Manual DLL staging must be the first substantive content in a dedicated flat if(MSVC) block")
endif()

string(SUBSTRING "${cmake_code}" ${command_start} -1 after_staging_command)
string(FIND "${after_staging_command}" "\nendif()" block_end_relative)
if(block_end_relative EQUAL -1)
    message(FATAL_ERROR "Could not isolate the dedicated flat if(MSVC) staging block")
endif()
string(SUBSTRING "${after_staging_command}" 0 ${block_end_relative} staging_body)

string(REGEX MATCH "(^|[\r\n])[ \t]*(if|else|elseif|endif)[ \t]*\\(" nested_branch "${staging_body}")
if(nested_branch)
    message(FATAL_ERROR "Dedicated MSVC DLL staging block must remain flat")
endif()

string(FIND "${staging_body}" "$<TARGET_RUNTIME_DLLS:Main>" body_runtime_position)
if(body_runtime_position EQUAL -1)
    message(FATAL_ERROR "$<TARGET_RUNTIME_DLLS:Main> must remain inside the dedicated flat MSVC block")
endif()

string(REGEX MATCHALL "copy_if_different" staging_copy_commands "${staging_body}")
list(LENGTH staging_copy_commands staging_copy_count)
if(NOT staging_copy_count EQUAL 1)
    message(FATAL_ERROR
        "Expected exactly one MSVC staging copy_if_different command, found ${staging_copy_count}")
endif()

string(REGEX MATCHALL "add_custom_command[ \t\r\n]*\\(" staging_custom_commands "${staging_body}")
list(LENGTH staging_custom_commands staging_custom_count)
if(NOT staging_custom_count EQUAL 1)
    message(FATAL_ERROR
        "Expected exactly one MSVC staging add_custom_command, found ${staging_custom_count}")
endif()

string(REGEX MATCH
    "add_custom_command[ \t\r\n]*\\([^)]*[\r\n][ \t]*\\)"
    staging_command "${staging_body}")
if(NOT staging_command)
    message(FATAL_ERROR "Could not isolate the combined MSVC DLL staging command")
endif()

require_match("${staging_command}"
    "add_custom_command[ \t\r\n]*\\([ \t\r\n]*TARGET[ \t]+Main[ \t]+POST_BUILD"
    "transitive staging is a Main POST_BUILD command")
require_match("${staging_command}"
    "COMMAND[ \t\r\n]+\"?\\$\\{CMAKE_COMMAND\\}\"?[ \t\r\n]+-E[ \t\r\n]+copy_if_different[ \t\r\n]+\\$<TARGET_RUNTIME_DLLS:Main>[ \t\r\n]+\"?\\$<TARGET_FILE_DIR:Main>\"?"
    "one copy command stages only linked-target DLLs beside Main")
require_match("${staging_command}" "COMMAND_EXPAND_LISTS" "transitive staging expands the DLL list")
require_match("${staging_command}" "VERBATIM" "transitive staging remains VERBATIM")

string(FIND "${cmake_code}" "set(ASSETS_STAMP" assets_anchor)
if(assets_anchor EQUAL -1)
    message(FATAL_ERROR "Missing asset staging anchor")
endif()
string(SUBSTRING "${cmake_code}" ${assets_anchor} -1 after_assets_anchor)
string(FIND "${after_assets_anchor}" "add_custom_target(CopyAssets" assets_end)
if(assets_end EQUAL -1)
    message(FATAL_ERROR "Could not isolate asset staging command")
endif()
string(SUBSTRING "${after_assets_anchor}" 0 ${assets_end} assets_command)
require_match("${assets_command}"
    "-DMU_ASSET_SOURCE=\\$\\{ASSETS_DIR\\}"
    "asset copy passes its source directory to the filtering helper")
require_match("${assets_command}"
    "-DMU_ASSET_OUTPUT=\\$<TARGET_FILE_DIR:Main>"
    "asset copy passes Main's runtime directory to the filtering helper")
require_match("${assets_command}"
    "\"?-P\"?[ \t\r\n]+\"?\\$\\{REPO_ROOT\\}/cmake/CopyRuntimeAssets\\.cmake\"?"
    "asset copy uses the runtime filtering helper")
reject_match("${assets_command}" "copy_directory" "unfiltered bulk asset copy")

get_filename_component(src_directory "${MU_CMAKE_SOURCE}" DIRECTORY)
get_filename_component(repo_root "${src_directory}" DIRECTORY)
set(asset_helper "${repo_root}/cmake/CopyRuntimeAssets.cmake")
if(NOT EXISTS "${asset_helper}")
    message(FATAL_ERROR "Missing runtime asset filtering helper: ${asset_helper}")
endif()
file(READ "${asset_helper}" asset_helper_source)
string(REGEX REPLACE "#[^\r\n]*" "" asset_helper_code "${asset_helper_source}")
require_match("${asset_helper_code}"
    "file[ \t\r\n]*\\([ \t\r\n]*GLOB_RECURSE[ \t\r\n]+asset_dlls[ \t\r\n]+LIST_DIRECTORIES[ \t\r\n]+FALSE[ \t\r\n]+RELATIVE[ \t\r\n]+\"\\$\\{MU_ASSET_SOURCE\\}\"[ \t\r\n]+\"\\$\\{MU_ASSET_SOURCE\\}/\\*\\.dll\""
    "asset staging finds asset DLL paths recursively")
require_match("${asset_helper_code}"
    "foreach[ \t\r\n]*\\([ \t\r\n]*asset_dll[ \t\r\n]+IN[ \t\r\n]+LISTS[ \t\r\n]+asset_dlls[ \t\r\n]*\\)"
    "asset staging iterates all asset DLL paths")
require_match("${asset_helper_code}"
    "file[ \t\r\n]*\\([ \t\r\n]*REMOVE[ \t\r\n]+\"\\$\\{MU_ASSET_OUTPUT\\}/\\$\\{asset_dll\\}\"[ \t\r\n]*\\)"
    "asset staging removes stale copies of asset DLLs only")
require_match("${asset_helper_code}"
    "file[ \t\r\n]*\\([ \t\r\n]*COPY[ \t\r\n]+\"\\$\\{MU_ASSET_SOURCE\\}/\"[ \t\r\n]+DESTINATION[ \t\r\n]+\"\\$\\{MU_ASSET_OUTPUT\\}\"[ \t\r\n]+PATTERN[ \t\r\n]+\"\\*\\.dll\"[ \t\r\n]+EXCLUDE"
    "asset staging excludes DLLs from the bulk copy")

message(STATUS "Validated runtime asset filtering and native MSVC DLL staging source contract")
