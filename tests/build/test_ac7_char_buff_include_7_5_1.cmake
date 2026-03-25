# Story 7.5.1: AC-7 — ZzzInfomation.cpp includes _GlobalFunctions.h for g_isCharacterBuff
# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS
#
# RED PHASE:  Test FAILS before story 7.5.1 is implemented.
#             ZzzInfomation.cpp uses g_isCharacterBuff() (declared in _GlobalFunctions.h)
#             but does not include that header, causing an undeclared identifier error on macOS.
#
# GREEN PHASE: Test PASSES after fix.
#             #include "_GlobalFunctions.h" added to ZzzInfomation.cpp.
#
# Validates:
#   AC-7 — _GlobalFunctions.h is included in ZzzInfomation.cpp

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of ZzzInfomation.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-7 FAIL: ZzzInfomation.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Verify that the file uses g_isCharacterBuff (confirm this is the right file)
string(FIND "${source_content}" "g_isCharacterBuff" usage_pos)
if(usage_pos EQUAL -1)
    message(WARNING "AC-7 NOTE: g_isCharacterBuff not found in ZzzInfomation.cpp — may have been refactored")
endif()

# Check that _GlobalFunctions.h is included (with or without Core/ prefix)
string(REGEX MATCH "#include[ \t]+\"(Core/)?_GlobalFunctions\\.h\"" include_found "${source_content}")
if(NOT include_found)
    message(FATAL_ERROR
        "AC-7 FAIL: '#include \"_GlobalFunctions.h\"' not found in ZzzInfomation.cpp.\n"
        "g_isCharacterBuff is declared in Core/_GlobalFunctions.h but the include is missing.\n"
        "Fix: Add '#include \"Core/_GlobalFunctions.h\"' to ZzzInfomation.cpp.\n"
        "Follow SortIncludes: Never — place after existing includes, preserving order.")
endif()

message(STATUS "AC-7 PASS: _GlobalFunctions.h included in ZzzInfomation.cpp — g_isCharacterBuff declared")
