# Story 7.5.1: AC-2 — ZzzOpenData.cpp MODEL_TYPE_CHARM_MIXWING uses static_cast<int>
# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS
#
# RED PHASE:  Test FAILS before story 7.5.1 is implemented.
#             ZzzOpenData.cpp uses MODEL_TYPE_CHARM_MIXWING + EWS_* without cast,
#             triggering -Wdeprecated-anon-enum-enum-conversion on Clang 14+.
#
# GREEN PHASE: Test PASSES after fix.
#             static_cast<int>(MODEL_TYPE_CHARM_MIXWING) applied at all ~20 call sites.
#
# Validates:
#   AC-2 — no bare MODEL_TYPE_CHARM_MIXWING + EWS_* arithmetic in ZzzOpenData.cpp

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of ZzzOpenData.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-2 FAIL: ZzzOpenData.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Check for un-cast enum arithmetic: MODEL_TYPE_CHARM_MIXWING directly followed by +
# The bad pattern is: MODEL_TYPE_CHARM_MIXWING + EWS_* without static_cast<int>()
string(REGEX MATCH "MODEL_TYPE_CHARM_MIXWING[ \t]*\\+" bare_arith "${source_content}")
if(bare_arith)
    message(FATAL_ERROR
        "AC-2 FAIL: MODEL_TYPE_CHARM_MIXWING used without static_cast<int> in ZzzOpenData.cpp.\n"
        "All call sites using MODEL_TYPE_CHARM_MIXWING + EWS_* must use the cast form:\n"
        "  static_cast<int>(MODEL_TYPE_CHARM_MIXWING) + EWS_KNIGHT_1_CHARM\n"
        "This eliminates -Wdeprecated-anon-enum-enum-conversion warnings on Clang 14+.")
endif()

# Verify static_cast<int>(MODEL_TYPE_CHARM_MIXWING) is present as the fix
string(FIND "${source_content}" "static_cast<int>(MODEL_TYPE_CHARM_MIXWING)" cast_pos)
if(cast_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-2 FAIL: static_cast<int>(MODEL_TYPE_CHARM_MIXWING) not found in ZzzOpenData.cpp.\n"
        "All ~20 call sites in AccessModel() and OpenTexture() must use the cast form.")
endif()

message(STATUS "AC-2 PASS: static_cast<int>(MODEL_TYPE_CHARM_MIXWING) used — no bare enum arithmetic in ZzzOpenData.cpp")
