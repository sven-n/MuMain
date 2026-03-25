# Story 7.5.1: AC-6 — ZzzInfomation.cpp sign comparison between DWORD and PET_TYPE fixed
# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS
#
# RED PHASE:  Test FAILS before story 7.5.1 is implemented.
#             ZzzInfomation.cpp:2272 compares m_dwPetType (DWORD, unsigned) with PET_TYPE_NONE
#             (signed enum value -1), triggering -Wsign-compare on Clang.
#
# GREEN PHASE: Test PASSES after fix.
#             A cast resolves the sign mismatch: static_cast<int>(pPetInfo->m_dwPetType)
#             or equivalent pattern used for the comparison.
#
# Validates:
#   AC-6 — direct DWORD vs PET_TYPE comparison at line 2272 has been fixed with a cast

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of ZzzInfomation.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-6 FAIL: ZzzInfomation.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Check for the bare sign-comparison pattern: m_dwPetType == PET_TYPE_NONE without cast
# The problematic form: pPetInfo->m_dwPetType == PET_TYPE_NONE (DWORD vs signed enum -1)
string(REGEX MATCH
    "m_dwPetType[ \t]*==[ \t]*PET_TYPE_NONE"
    bare_compare
    "${source_content}")
if(bare_compare)
    message(FATAL_ERROR
        "AC-6 FAIL: Bare sign comparison 'm_dwPetType == PET_TYPE_NONE' in ZzzInfomation.cpp.\n"
        "m_dwPetType is DWORD (unsigned); PET_TYPE_NONE is -1 (signed enum).\n"
        "Fix: Use static_cast<int>(pPetInfo->m_dwPetType) == PET_TYPE_NONE\n"
        "  or static_cast<DWORD>(PET_TYPE_NONE) for the comparison.\n"
        "Note: m_dwPetType stores PET_TYPE_NONE as 0xFFFFFFFF (DWORD cast of -1).")
endif()

message(STATUS "AC-6 PASS: Sign comparison between m_dwPetType and PET_TYPE_NONE resolved in ZzzInfomation.cpp")
