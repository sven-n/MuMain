cmake_minimum_required(VERSION 3.25)

# Fails (non-zero exit via FATAL_ERROR) if any control socket source was
# compiled into the client while ENABLE_CONTROL_SOCKET=OFF. Run by CTest as:
#   cmake -DSOURCES_FILE=<file> -DCONTROL_DIR=<dir> -DTRANSPORT_FILE=<file>
#         -P test_control_leak.cmake
# SOURCES_FILE is produced by file(GENERATE) and holds one source path per
# line: the resolved SOURCES list of the target that owns the client code.
# CONTROL_DIR is the absolute path of src/source/App/Control; TRANSPORT_FILE
# the absolute path of the local-socket transport that only the control
# socket uses.

foreach(var SOURCES_FILE CONTROL_DIR TRANSPORT_FILE)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "${var} must be passed with -D${var}=<path>")
    endif()
endforeach()

file(STRINGS "${SOURCES_FILE}" sources)

set(leaked "")
foreach(src IN LISTS sources)
    # A path-segment containment test (not a substring match), so it flags
    # files inside CONTROL_DIR without false-positiving on an unrelated
    # directory that happens to be named "Control".
    cmake_path(IS_PREFIX CONTROL_DIR "${src}" NORMALIZE inside)
    if(inside)
        list(APPEND leaked "${src}")
        continue()
    endif()
    cmake_path(COMPARE "${src}" EQUAL "${TRANSPORT_FILE}" is_transport)
    if(is_transport)
        list(APPEND leaked "${src}")
    endif()
endforeach()

if(leaked)
    string(REPLACE ";" "\n  - " leaked "${leaked}")
    message(FATAL_ERROR
        "control socket sources leaked into the build with ENABLE_CONTROL_SOCKET=OFF:\n  - ${leaked}")
endif()

message(STATUS "PASS: no control socket sources compiled with ENABLE_CONTROL_SOCKET=OFF.")
