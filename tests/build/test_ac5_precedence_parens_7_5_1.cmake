# Story 7.5.1: AC-5 — ZzzInfomation.cpp && within || precedence warnings resolved
# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS
#
# RED PHASE:  Test FAILS before story 7.5.1 is implemented.
#             ZzzInfomation.cpp lines 754, 1115, 1791 have && operators within || chains
#             without explicit outer parentheses, triggering -Wlogical-op-parentheses on Clang.
#             Line 754 also has a tautological range overlap.
#
# GREEN PHASE: Test PASSES after fix.
#             Explicit parentheses added around && sub-expressions in || chains.
#             Tautological comparison at line 754 corrected.
#
# Validates:
#   AC-5 — the specific ambiguous && within || pattern at line 754 has explicit outer parens

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of ZzzInfomation.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-5 FAIL: ZzzInfomation.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# Check for the specific ambiguous pattern at line 754:
# ... || (ip->Type >= ITEM_WING_OF_DIMENSION && ip->Type < ITEM_CAPE_OF_EMPEROR) && (ip->Type != ITEM_CAPE_OF_FIGHTER)
# The tautological tautology: range [ITEM_WING_OF_DIMENSION, ITEM_CAPE_OF_EMPEROR) already
# excludes ITEM_CAPE_OF_FIGHTER if ITEM_CAPE_OF_FIGHTER >= ITEM_CAPE_OF_EMPEROR.
# After fix: this last OR clause should be wrapped in extra parens or simplified.
string(REGEX MATCH
    "ITEM_CAPE_OF_EMPEROR\\)[ \t]*&&[ \t]*\\(ip->Type[ \t]*!=[ \t]*ITEM_CAPE_OF_FIGHTER\\)"
    ambiguous_pattern
    "${source_content}")
if(ambiguous_pattern)
    message(FATAL_ERROR
        "AC-5 FAIL: Ambiguous && within || at line 754 in ZzzInfomation.cpp — no explicit outer parens.\n"
        "Pattern found: '${ambiguous_pattern}'\n"
        "Fix: Wrap the complete last || operand in explicit parentheses:\n"
        "  || ((ip->Type >= ITEM_WING_OF_DIMENSION && ip->Type < ITEM_CAPE_OF_EMPEROR)\n"
        "      && (ip->Type != ITEM_CAPE_OF_FIGHTER))\n"
        "Also verify: if ITEM_CAPE_OF_FIGHTER < ITEM_CAPE_OF_EMPEROR, the != check is tautological\n"
        "and the expression can be simplified to remove it.")
endif()

message(STATUS "AC-5 PASS: No ambiguous && within || without explicit parens at line 754 in ZzzInfomation.cpp")
