# Story 7.6.1: AC-8 — xstreambuf.cpp delete void* fixed (3 occurrences)
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if xstreambuf.cpp deletes m_pBuffer as a raw void* without casting.
#             delete void* is undefined behavior in C++ (compiler cannot determine destructor/size).
#             Clang on macOS treats this as an error; MSVC historically accepted it as a warning.
#
# GREEN PHASE: Test PASSES when:
#   - All delete[] on m_pBuffer use static_cast<char*> (or another typed cast) before deletion
#   - No bare "delete[] m_pBuffer" or "delete m_pBuffer" without a typed cast
#
# Validates:
#   AC-8 — xstreambuf.cpp void* deletion fixed with typed cast in all 3 locations

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of xstreambuf.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-8 FAIL: xstreambuf.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Check for any unguarded "delete m_pBuffer" without a cast
# Regex: delete followed by optional [] then whitespace then m_pBuffer without a cast
# Safe patterns: delete[] static_cast<char*>(m_pBuffer) or delete[] static_cast<BYTE*>(m_pBuffer)
# Unsafe patterns: delete[] m_pBuffer or delete m_pBuffer

# Strip safe patterns (delete with cast) to detect unsafe remaining ones
string(REPLACE "delete[] static_cast<char*>(m_pBuffer)" "_SAFE_DELETE_" cleaned "${source_content}")
string(REPLACE "delete[] static_cast<BYTE*>(m_pBuffer)" "_SAFE_DELETE_" cleaned "${cleaned}")
string(REPLACE "delete[] static_cast<unsigned char*>(m_pBuffer)" "_SAFE_DELETE_" cleaned "${cleaned}")
string(REPLACE "delete static_cast<char*>(m_pBuffer)" "_SAFE_DELETE_" cleaned "${cleaned}")
string(REPLACE "free(m_pBuffer)" "_SAFE_DELETE_" cleaned "${cleaned}")

# Now check for any remaining untyped delete of m_pBuffer
string(REGEX MATCH "delete\\[\\][ \t]*m_pBuffer" bad_delete_arr "${cleaned}")
string(REGEX MATCH "delete[ \t]+m_pBuffer" bad_delete_single "${cleaned}")

if(bad_delete_arr OR bad_delete_single)
    message(FATAL_ERROR
        "AC-8 FAIL: xstreambuf.cpp contains untyped 'delete m_pBuffer' without a type cast.\n"
        "Deleting a void* is undefined behavior in C++ (C++ standard §8.3.5).\n"
        "Clang on macOS rejects this; MSVC historically accepted it.\n"
        "Fix: Replace 'delete[] m_pBuffer' with 'delete[] static_cast<char*>(m_pBuffer)'\n"
        "     in all 3 destructor/resize locations in xstreambuf.cpp")
endif()

message(STATUS "AC-8 PASS: xstreambuf.cpp delete void* fixed — all m_pBuffer deletions use typed cast")
