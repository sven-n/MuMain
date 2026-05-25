#pragma once

// X-Macro definition for all ITEM_ATTRIBUTE fields (except Name which is special)
// Format: X(FieldName, TypeEnum, ArraySize, DefaultColumnWidth, I18nMetadataName)
// I18nMetadataName is the bare identifier inside namespace I18N::Metadata that
// holds the localized display label for this field.
// This is the SINGLE SOURCE OF TRUTH for field definitions.

#define ITEM_FIELDS_SIMPLE(X) \
    X(TwoHand, Bool, 1, 70.0f, TwoHand) \
    X(Level, Word, 1, 60.0f, Level) \
    X(m_byItemSlot, Byte, 1, 60.0f, Slot) \
    X(m_wSkillIndex, Word, 1, 60.0f, Skill) \
    X(Width, Byte, 1, 60.0f, Width) \
    X(Height, Byte, 1, 60.0f, Height) \
    X(DamageMin, Byte, 1, 70.0f, MinDamage) \
    X(DamageMax, Byte, 1, 70.0f, MaxDamage) \
    X(SuccessfulBlocking, Byte, 1, 70.0f, BlockRate) \
    X(Defense, Byte, 1, 70.0f, Defense) \
    X(MagicDefense, Byte, 1, 80.0f, MagicDefense) \
    X(WeaponSpeed, Byte, 1, 80.0f, AttackSpeed) \
    X(WalkSpeed, Byte, 1, 80.0f, MoveSpeed) \
    X(Durability, Byte, 1, 70.0f, Durability) \
    X(MagicDur, Byte, 1, 90.0f, MagicDurability) \
    X(MagicPower, Byte, 1, 80.0f, MagicPower) \
    X(RequireStrength, Word, 1, 80.0f, ReqStrength) \
    X(RequireDexterity, Word, 1, 80.0f, ReqDexterity) \
    X(RequireEnergy, Word, 1, 80.0f, ReqEnergy) \
    X(RequireVitality, Word, 1, 80.0f, ReqVitality) \
    X(RequireCharisma, Word, 1, 90.0f, ReqLeadership) \
    X(RequireLevel, Word, 1, 70.0f, ReqLevel) \
    X(Value, Byte, 1, 70.0f, SellValue) \
    X(iZen, Int, 1, 80.0f, BuyPrice) \
    X(AttType, Byte, 1, 80.0f, AttackType)

// Array fields defined with index notation for metadata
// Format: X(FieldNameWithIndex, BaseFieldName, Index, TypeEnum, DefaultColumnWidth, I18nMetadataName)
#define ITEM_FIELDS_ARRAYS(X) \
    X(RequireClass[0], RequireClass, 0, Byte, 60.0f, DWSM) \
    X(RequireClass[1], RequireClass, 1, Byte, 60.0f, DKBK) \
    X(RequireClass[2], RequireClass, 2, Byte, 65.0f, ELFME) \
    X(RequireClass[3], RequireClass, 3, Byte, 60.0f, MGDM) \
    X(RequireClass[4], RequireClass, 4, Byte, 60.0f, DLLE) \
    X(RequireClass[5], RequireClass, 5, Byte, 65.0f, SUMBS) \
    X(RequireClass[6], RequireClass, 6, Byte, 60.0f, RFFM) \
    X(Resistance[0], Resistance, 0, Byte, 65.0f, IceResistance) \
    X(Resistance[1], Resistance, 1, Byte, 75.0f, PoisonResistance) \
    X(Resistance[2], Resistance, 2, Byte, 85.0f, LightningResistance) \
    X(Resistance[3], Resistance, 3, Byte, 65.0f, FireResistance) \
    X(Resistance[4], Resistance, 4, Byte, 70.0f, EarthResistance) \
    X(Resistance[5], Resistance, 5, Byte, 70.0f, WindResistance) \
    X(Resistance[6], Resistance, 6, Byte, 75.0f, WaterResistance) \
    X(Resistance[7], Resistance, 7, Byte, 60.0f, Resistance7)

// Helper macro to convert TypeEnum to actual C++ type
#define FIELD_TYPE_Bool bool
#define FIELD_TYPE_Byte BYTE
#define FIELD_TYPE_Word WORD
#define FIELD_TYPE_Int int
#define FIELD_TYPE_DWord DWORD

// Generate struct field declarations from X-macro. The i18nName parameter
// only matters for descriptor generation; the struct layout ignores it.
#define DECLARE_FIELD(name, type, arraySize, width, i18nName) FIELD_TYPE_##type name;

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
