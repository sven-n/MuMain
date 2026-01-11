#pragma once

// X-Macro definition for all ITEM_ATTRIBUTE fields (except Name which is special)
// Format: X(FieldName, TypeEnum, ArraySize, DefaultColumnWidth)
// This is the SINGLE SOURCE OF TRUTH for field definitions

#define ITEM_FIELDS_SIMPLE(X) \
    X(TwoHand, Bool, 1, 70.0f) \
    X(Level, Word, 1, 60.0f) \
    X(m_byItemSlot, Byte, 1, 60.0f) \
    X(m_wSkillIndex, Word, 1, 60.0f) \
    X(Width, Byte, 1, 60.0f) \
    X(Height, Byte, 1, 60.0f) \
    X(DamageMin, Byte, 1, 70.0f) \
    X(DamageMax, Byte, 1, 70.0f) \
    X(SuccessfulBlocking, Byte, 1, 70.0f) \
    X(Defense, Byte, 1, 70.0f) \
    X(MagicDefense, Byte, 1, 80.0f) \
    X(WeaponSpeed, Byte, 1, 80.0f) \
    X(WalkSpeed, Byte, 1, 80.0f) \
    X(Durability, Byte, 1, 70.0f) \
    X(MagicDur, Byte, 1, 90.0f) \
    X(MagicPower, Byte, 1, 80.0f) \
    X(RequireStrength, Word, 1, 80.0f) \
    X(RequireDexterity, Word, 1, 80.0f) \
    X(RequireEnergy, Word, 1, 80.0f) \
    X(RequireVitality, Word, 1, 80.0f) \
    X(RequireCharisma, Word, 1, 90.0f) \
    X(RequireLevel, Word, 1, 70.0f) \
    X(Value, Byte, 1, 70.0f) \
    X(iZen, Int, 1, 80.0f) \
    X(AttType, Byte, 1, 80.0f)

// Array fields defined with index notation for metadata
// Format: X(FieldNameWithIndex, BaseFieldName, Index, TypeEnum, DefaultColumnWidth)
// No hardcoded display names - translations come from metadata.json!
#define ITEM_FIELDS_ARRAYS(X) \
    X(RequireClass[0], RequireClass, 0, Byte, 60.0f) \
    X(RequireClass[1], RequireClass, 1, Byte, 60.0f) \
    X(RequireClass[2], RequireClass, 2, Byte, 65.0f) \
    X(RequireClass[3], RequireClass, 3, Byte, 60.0f) \
    X(RequireClass[4], RequireClass, 4, Byte, 60.0f) \
    X(RequireClass[5], RequireClass, 5, Byte, 65.0f) \
    X(RequireClass[6], RequireClass, 6, Byte, 60.0f) \
    X(Resistance[0], Resistance, 0, Byte, 65.0f) \
    X(Resistance[1], Resistance, 1, Byte, 75.0f) \
    X(Resistance[2], Resistance, 2, Byte, 85.0f) \
    X(Resistance[3], Resistance, 3, Byte, 65.0f) \
    X(Resistance[4], Resistance, 4, Byte, 70.0f) \
    X(Resistance[5], Resistance, 5, Byte, 70.0f) \
    X(Resistance[6], Resistance, 6, Byte, 75.0f) \
    X(Resistance[7], Resistance, 7, Byte, 60.0f)

// Helper macro to convert TypeEnum to actual C++ type
#define FIELD_TYPE_Bool bool
#define FIELD_TYPE_Byte BYTE
#define FIELD_TYPE_Word WORD
#define FIELD_TYPE_Int int
#define FIELD_TYPE_DWord DWORD

// Generate struct field declarations from X-macro
#define DECLARE_FIELD(name, type, arraySize, width) FIELD_TYPE_##type name;

// Macro to generate all non-array fields for struct definition
#define ITEM_ATTRIBUTE_FIELDS \
    ITEM_FIELDS_SIMPLE(DECLARE_FIELD) \
    BYTE RequireClass[MAX_CLASS]; \
    BYTE Resistance[MAX_RESISTANCE + 1];

// ============================================================================
// EDITOR METADATA - Defined in ItemFieldMetadata.h (after ITEM_ATTRIBUTE is defined)
// ============================================================================
// Note: Editor-specific types (EItemFieldType, FieldDescriptor, etc.) are defined
// in ItemFieldMetadata.h to avoid circular dependencies
