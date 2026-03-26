# Flow Code: VS0-QUAL-BUILD-HEADERS
# Story 7.8.2: Gameplay Header Cross-Platform Fixes
# AC-1: Core/mu_enum.h — SKILL_REPLACEMENTS must be declared with 'inline' keyword
#
# Without 'inline', including mu_enum.h from multiple translation units causes an
# ODR (One Definition Rule) violation: the linker sees multiple definitions of the
# same const std::map variable and fails on macOS/Linux.
#
# RED PHASE: 'inline' absent → ODR linker error on macOS/Linux when MuTests builds
# GREEN PHASE: 'inline' present → each TU gets its own copy, linker succeeds

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED MU_ENUM_H)
    message(FATAL_ERROR "AC-1: MU_ENUM_H variable not defined")
endif()

if(NOT EXISTS "${MU_ENUM_H}")
    message(FATAL_ERROR "AC-1: mu_enum.h not found at: ${MU_ENUM_H}")
endif()

file(READ "${MU_ENUM_H}" content)

# ---------------------------------------------------------------------------
# Check 1: 'inline' keyword must appear on the SKILL_REPLACEMENTS declaration
# In RED phase: "const std::map<ActionSkillType" is present without 'inline' → FAIL
# In GREEN phase: "inline const std::map<ActionSkillType" is present → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "inline const std::map<ActionSkillType, ActionSkillType> SKILL_REPLACEMENTS" pos_inline)
if(pos_inline EQUAL -1)
    # Also reject the non-inline form to give a clear failure message
    string(FIND "${content}" "const std::map<ActionSkillType, ActionSkillType> SKILL_REPLACEMENTS" pos_no_inline)
    if(NOT pos_no_inline EQUAL -1)
        message(FATAL_ERROR
            "AC-1 FAILED: SKILL_REPLACEMENTS is declared without 'inline' in mu_enum.h\n"
            "Fix: Change:\n"
            "  const std::map<ActionSkillType, ActionSkillType> SKILL_REPLACEMENTS = {...};\n"
            "To:\n"
            "  inline const std::map<ActionSkillType, ActionSkillType> SKILL_REPLACEMENTS = {...};\n"
            "Why: Without 'inline', multiple TUs including this header cause an ODR violation\n"
            "     (duplicate symbol linker error) on macOS/Linux.")
    else()
        message(FATAL_ERROR
            "AC-1 FAILED: SKILL_REPLACEMENTS declaration not found in mu_enum.h.\n"
            "Expected: 'inline const std::map<ActionSkillType, ActionSkillType> SKILL_REPLACEMENTS'")
    endif()
endif()

# ---------------------------------------------------------------------------
# Regression guard: verify file is non-trivial
# ---------------------------------------------------------------------------
string(LENGTH "${content}" content_len)
if(content_len LESS 1000)
    message(FATAL_ERROR "AC-1: mu_enum.h appears too short (${content_len} bytes) — file may be truncated")
endif()

message(STATUS "AC-1 PASSED: SKILL_REPLACEMENTS in mu_enum.h is correctly declared with 'inline' (ODR-safe)")
