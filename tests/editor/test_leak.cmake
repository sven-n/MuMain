# Fails (non-zero exit via FATAL_ERROR) if any source under the editor
# directory was compiled into the client while ENABLE_EDITOR=OFF. Run by
# CTest as:
#   cmake -DSOURCES_FILE=<file> -DEDITOR_DIR=<dir> -P test_leak.cmake
# SOURCES_FILE is produced by file(GENERATE) and holds one source path per
# line: the resolved SOURCES list of the target that owns the editor code.
# EDITOR_DIR is the absolute path of the editor source directory.

foreach(var SOURCES_FILE EDITOR_DIR)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "${var} must be passed with -D${var}=<path>")
    endif()
endforeach()

file(STRINGS "${SOURCES_FILE}" sources)

set(leaked "")
foreach(src IN LISTS sources)
    # A path-segment containment test (not a substring match), so it flags
    # files inside EDITOR_DIR without false-positiving when the repository
    # itself is checked out under an unrelated directory named "MuEditor".
    cmake_path(IS_PREFIX EDITOR_DIR "${src}" NORMALIZE inside)
    if(inside)
        list(APPEND leaked "${src}")
    endif()
endforeach()

if(leaked)
    string(REPLACE ";" "\n  - " leaked "${leaked}")
    message(FATAL_ERROR
        "MuEditor leaked into the build with ENABLE_EDITOR=OFF:\n  - ${leaked}")
endif()

message(STATUS "PASS: no MuEditor sources compiled with ENABLE_EDITOR=OFF.")
