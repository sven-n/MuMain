# Flow Code: VS0-QUAL-BUILD-HEADERS
# Story 7.8.2: Gameplay Header Cross-Platform Fixes
# AC-4: Gameplay/Items/CSItemOption.h — ActionSkillType and ITEM must be properly included
#
# CSItemOption.h declares static methods that use ActionSkillType and ITEM* as parameter
# types, but includes only: Singleton.h, <array>, <cstdint>, <map>.
# Neither ActionSkillType (defined in mu_enum.h) nor ITEM (defined in mu_struct.h or
# equivalent) are included. On macOS/Linux with fresh include paths, this causes
# compilation failures when CSItemOption.h is included without these types in scope.
#
# RED PHASE: Required type includes absent → ActionSkillType/ITEM undefined → FAIL
# GREEN PHASE: Required includes present → CSItemOption.h is self-contained → PASS

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CS_ITEM_OPTION_H)
    message(FATAL_ERROR "AC-4: CS_ITEM_OPTION_H variable not defined")
endif()

if(NOT EXISTS "${CS_ITEM_OPTION_H}")
    message(FATAL_ERROR "AC-4: CSItemOption.h not found at: ${CS_ITEM_OPTION_H}")
endif()

file(READ "${CS_ITEM_OPTION_H}" content)

# ---------------------------------------------------------------------------
# Check 1: ActionSkillType must be declared before use
# Either via #include "mu_enum.h" or a forward declaration.
# In RED phase: neither present → FAIL
# In GREEN phase: include or forward declaration present → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "#include \"mu_enum.h\"" pos_mu_enum)
string(FIND "${content}" "enum ActionSkillType" pos_fwd_action)
string(FIND "${content}" "enum class ActionSkillType" pos_fwd_action_class)

if(pos_mu_enum EQUAL -1 AND pos_fwd_action EQUAL -1 AND pos_fwd_action_class EQUAL -1)
    message(FATAL_ERROR
        "AC-4 FAILED: CSItemOption.h uses ActionSkillType but does not include mu_enum.h\n"
        "and has no forward declaration for ActionSkillType.\n"
        "Fix: Add '#include \"mu_enum.h\"' to CSItemOption.h\n"
        "Why: IsDisableSkill() and IsNonWeaponSkillOrIsSkillEquipped() take ActionSkillType\n"
        "     parameters. Without mu_enum.h in scope, ActionSkillType is undefined.")
endif()

# ---------------------------------------------------------------------------
# Check 2: ITEM type must be declared before use
# ITEM is defined in mu_struct.h (or equivalent). Accept include or forward declaration.
# In RED phase: neither present → FAIL
# In GREEN phase: include or forward declaration present → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "#include \"mu_struct.h\"" pos_mu_struct)
string(FIND "${content}" "#include \"Item.h\"" pos_item_h)
string(FIND "${content}" "struct ITEM" pos_fwd_item)

if(pos_mu_struct EQUAL -1 AND pos_item_h EQUAL -1 AND pos_fwd_item EQUAL -1)
    message(FATAL_ERROR
        "AC-4 FAILED: CSItemOption.h uses ITEM* but does not include the header\n"
        "that defines struct ITEM and has no forward declaration for ITEM.\n"
        "Fix: Add '#include \"mu_struct.h\"' (or forward-declare 'struct ITEM;') to CSItemOption.h\n"
        "Why: GetDefaultOptionValue(), GetDefaultOptionText(), RenderDefaultOptionText()\n"
        "     and similar methods take ITEM* parameters. Without the definition in scope,\n"
        "     the compiler cannot resolve ITEM.")
endif()

# ---------------------------------------------------------------------------
# Verify ActionSkillType and ITEM usages are present (regression guard)
# If methods were removed rather than fixed, this warns.
# ---------------------------------------------------------------------------
string(FIND "${content}" "ActionSkillType" pos_action_usage)
if(pos_action_usage EQUAL -1)
    message(WARNING "AC-4: ActionSkillType not found in CSItemOption.h — may have been removed")
endif()

string(FIND "${content}" "ITEM*" pos_item_ptr_usage)
if(pos_item_ptr_usage EQUAL -1)
    message(WARNING "AC-4: ITEM* not found in CSItemOption.h — may have been removed")
endif()

# ---------------------------------------------------------------------------
# Regression guard: file is non-trivial
# ---------------------------------------------------------------------------
string(LENGTH "${content}" content_len)
if(content_len LESS 500)
    message(FATAL_ERROR "AC-4: CSItemOption.h appears too short (${content_len} bytes) — file may be truncated")
endif()

message(STATUS "AC-4 PASSED: CSItemOption.h has explicit includes for ActionSkillType and ITEM types")
