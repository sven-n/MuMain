# Story 1.2.2 AC-4: Verify CMake platform-conditional backend selection
# RED PHASE: Will fail until CMakeLists.txt is updated with backend selection.

if(NOT DEFINED CMAKELISTS_FILE)
    message(FATAL_ERROR "CMAKELISTS_FILE not set")
endif()

if(NOT EXISTS "${CMAKELISTS_FILE}")
    message(FATAL_ERROR "AC-4 FAIL: CMakeLists.txt does not exist at ${CMAKELISTS_FILE}")
endif()

file(READ "${CMAKELISTS_FILE}" CMAKE_CONTENT)

# Verify win32 backend source is referenced
string(FIND "${CMAKE_CONTENT}" "win32/PlatformLibrary.cpp" WIN32_SRC_POS)
if(WIN32_SRC_POS EQUAL -1)
    message(FATAL_ERROR "AC-4 FAIL: CMakeLists.txt does not reference win32/PlatformLibrary.cpp")
endif()

# Verify posix backend source is referenced
string(FIND "${CMAKE_CONTENT}" "posix/PlatformLibrary.cpp" POSIX_SRC_POS)
if(POSIX_SRC_POS EQUAL -1)
    message(FATAL_ERROR "AC-4 FAIL: CMakeLists.txt does not reference posix/PlatformLibrary.cpp")
endif()

# Verify platform-conditional selection uses if(WIN32)
string(FIND "${CMAKE_CONTENT}" "if(WIN32)" IF_WIN32_POS)
if(IF_WIN32_POS EQUAL -1)
    message(FATAL_ERROR "AC-4 FAIL: CMakeLists.txt does not use if(WIN32) for backend selection")
endif()

# Verify backend source files exist
set(SRC_DIR "${CMAKELISTS_FILE}")
get_filename_component(SRC_DIR "${SRC_DIR}" DIRECTORY)
set(WIN32_BACKEND "${SRC_DIR}/source/Platform/win32/PlatformLibrary.cpp")
set(POSIX_BACKEND "${SRC_DIR}/source/Platform/posix/PlatformLibrary.cpp")

if(NOT EXISTS "${WIN32_BACKEND}")
    message(FATAL_ERROR "AC-2 FAIL: win32/PlatformLibrary.cpp does not exist at ${WIN32_BACKEND}")
endif()

if(NOT EXISTS "${POSIX_BACKEND}")
    message(FATAL_ERROR "AC-3 FAIL: posix/PlatformLibrary.cpp does not exist at ${POSIX_BACKEND}")
endif()

message(STATUS "AC-4 PASS: CMake uses platform-conditional backend selection")
message(STATUS "AC-2 PASS: win32/PlatformLibrary.cpp exists")
message(STATUS "AC-3 PASS: posix/PlatformLibrary.cpp exists")
