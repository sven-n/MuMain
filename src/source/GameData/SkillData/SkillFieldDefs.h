#pragma once

#include <windows.h>

// X-Macro definition for all SKILL_ATTRIBUTE fields (except Name which is special)
// Format: X(FieldName, TypeEnum, ArraySize, DefaultColumnWidth)
// This is the SINGLE SOURCE OF TRUTH for skill field definitions

#define SKILL_FIELDS_SIMPLE(X) \
    X(Level, Word, 1, 60.0f) \
    X(Damage, Word, 1, 70.0f) \
    X(Mana, Word, 1, 70.0f) \
    X(AbilityGuage, Word, 1, 80.0f) \
    X(Distance, DWord, 1, 80.0f) \
    X(Delay, Int, 1, 70.0f) \
    X(Energy, Int, 1, 70.0f) \
    X(Charisma, Word, 1, 80.0f) \
    X(MasteryType, Byte, 1, 80.0f) \
    X(SkillUseType, Byte, 1, 80.0f) \
    X(SkillBrand, DWord, 1, 90.0f) \
    X(KillCount, Byte, 1, 80.0f)

#define SKILL_FIELDS_ARRAYS(X) \
    X(RequireDutyClass[0], RequireDutyClass, 0, Byte, 80.0f) \
    X(RequireDutyClass[1], RequireDutyClass, 1, Byte, 80.0f) \
    X(RequireDutyClass[2], RequireDutyClass, 2, Byte, 80.0f) \
    X(RequireClass[0], RequireClass, 0, Byte, 60.0f) \
    X(RequireClass[1], RequireClass, 1, Byte, 60.0f) \
    X(RequireClass[2], RequireClass, 2, Byte, 65.0f) \
    X(RequireClass[3], RequireClass, 3, Byte, 60.0f) \
    X(RequireClass[4], RequireClass, 4, Byte, 60.0f) \
    X(RequireClass[5], RequireClass, 5, Byte, 65.0f) \
    X(RequireClass[6], RequireClass, 6, Byte, 60.0f)

// Helper macro to convert TypeEnum to actual C++ type (same as ItemFieldDefs.h)
#define SKILL_FIELD_TYPE_Bool bool
#define SKILL_FIELD_TYPE_Byte BYTE
#define SKILL_FIELD_TYPE_Word WORD
#define SKILL_FIELD_TYPE_Int int
#define SKILL_FIELD_TYPE_DWord DWORD

// Generate struct field declarations from X-macro
#define DECLARE_SKILL_FIELD(name, type, arraySize, width) SKILL_FIELD_TYPE_##type name;

// Fields that come after the arrays (must be in correct order for binary compatibility)
#define SKILL_FIELDS_AFTER_ARRAYS(X) \
    X(SkillRank, Byte, 1, 70.0f) \
    X(Magic_Icon, Word, 1, 80.0f) \
    X(TypeSkill, Byte, 1, 80.0f) \
    X(Strength, Int, 1, 80.0f) \
    X(Dexterity, Int, 1, 80.0f) \
    X(ItemSkill, Byte, 1, 80.0f) \
    X(IsDamage, Byte, 1, 70.0f) \
    X(Effect, Word, 1, 70.0f)

// Macro to generate all fields for struct definition (excludes Name)
// IMPORTANT: Field order must match original binary format!
#define SKILL_ATTRIBUTE_FIELDS \
    SKILL_FIELDS_SIMPLE(DECLARE_SKILL_FIELD) \
    BYTE RequireDutyClass[MAX_DUTY_CLASS]; \
    BYTE RequireClass[MAX_CLASS]; \
    SKILL_FIELDS_AFTER_ARRAYS(DECLARE_SKILL_FIELD)

// Note: WCharArray for Name field is handled specially in SkillFieldMetadata.h
