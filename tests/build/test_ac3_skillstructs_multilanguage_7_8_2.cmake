# Flow Code: VS0-QUAL-BUILD-HEADERS
# Story 7.8.2: Gameplay Header Cross-Platform Fixes
# AC-3: Data/Skills/SkillStructs.h — CMultiLanguage must be properly included
#
# SkillStructs.h uses CMultiLanguage::ConvertFromUtf8() and ConvertToUtf8() in
# inline helper functions but does not include MultiLanguage.h. The file already
# carries comments "// Requires: #include \"MultiLanguage.h\"" acknowledging the
# dependency, but the include is absent. On macOS/Linux this causes compilation
# failure when the header is included without MultiLanguage.h in scope.
#
# RED PHASE: #include "MultiLanguage.h" absent → CMultiLanguage undefined → FAIL
# GREEN PHASE: #include "MultiLanguage.h" present (or CMultiLanguage forward-declared) → PASS

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SKILL_STRUCTS_H)
    message(FATAL_ERROR "AC-3: SKILL_STRUCTS_H variable not defined")
endif()

if(NOT EXISTS "${SKILL_STRUCTS_H}")
    message(FATAL_ERROR "AC-3: SkillStructs.h not found at: ${SKILL_STRUCTS_H}")
endif()

file(READ "${SKILL_STRUCTS_H}" content)

# ---------------------------------------------------------------------------
# Check 1: SkillStructs.h must include MultiLanguage.h
# The file contains "// Requires: #include \"MultiLanguage.h\"" comments but the
# actual include is missing. The fix adds the include to make the header self-contained.
# In RED phase: no #include "MultiLanguage.h" → FAIL
# In GREEN phase: #include "MultiLanguage.h" present → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "#include \"MultiLanguage.h\"" pos_include)
string(FIND "${content}" "class CMultiLanguage" pos_fwd_decl)

if(pos_include EQUAL -1 AND pos_fwd_decl EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAILED: SkillStructs.h does not include MultiLanguage.h and does not\n"
        "forward-declare CMultiLanguage.\n"
        "Fix: Add '#include \"MultiLanguage.h\"' to SkillStructs.h\n"
        "Why: The inline helpers ConvertSkillAttributeFromUtf8/ToUtf8 call\n"
        "     CMultiLanguage::ConvertFromUtf8 and ConvertToUtf8 but the type is\n"
        "     undefined without MultiLanguage.h in scope. Comments in the file already\n"
        "     document this requirement.")
endif()

# ---------------------------------------------------------------------------
# Check 2: CMultiLanguage usage is still present (regression guard)
# If the inline helpers were removed rather than fixed, this check warns.
# ---------------------------------------------------------------------------
string(FIND "${content}" "CMultiLanguage::" pos_usage)
if(pos_usage EQUAL -1 AND NOT pos_fwd_decl EQUAL -1)
    message(WARNING
        "AC-3: CMultiLanguage::* usage not found in SkillStructs.h.\n"
        "If the inline helpers were removed instead of fixed, verify AC-3 intent.")
endif()

# ---------------------------------------------------------------------------
# Regression guard: file is non-trivial
# ---------------------------------------------------------------------------
string(LENGTH "${content}" content_len)
if(content_len LESS 500)
    message(FATAL_ERROR "AC-3: SkillStructs.h appears too short (${content_len} bytes) — file may be truncated")
endif()

message(STATUS "AC-3 PASSED: SkillStructs.h has explicit MultiLanguage.h dependency (CMultiLanguage is declared)")
