#pragma once

#ifdef _EDITOR

#include "DataHandler/FieldMetadata.h"
#include "GameData/ItemData/ItemStructs.h"

/**
 * Item field metadata for comparison - C++20 constexpr arrays
 *
 * ZERO MACROS! Crystal clear, explicit field definitions.
 * Every field is visible and easy to understand.
 */

namespace ItemComparisonMetadata
{
    // Simple (non-array) fields
    inline constexpr FieldInfo SIMPLE_FIELDS[] = {
        { "TwoHand", FieldType::Bool, offsetof(ITEM_ATTRIBUTE, TwoHand), sizeof(ITEM_ATTRIBUTE::TwoHand), -1 },
        { "Level", FieldType::Word, offsetof(ITEM_ATTRIBUTE, Level), sizeof(ITEM_ATTRIBUTE::Level), -1 },
        { "m_byItemSlot", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, m_byItemSlot), sizeof(ITEM_ATTRIBUTE::m_byItemSlot), -1 },
        { "m_wSkillIndex", FieldType::Word, offsetof(ITEM_ATTRIBUTE, m_wSkillIndex), sizeof(ITEM_ATTRIBUTE::m_wSkillIndex), -1 },
        { "Width", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Width), sizeof(ITEM_ATTRIBUTE::Width), -1 },
        { "Height", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Height), sizeof(ITEM_ATTRIBUTE::Height), -1 },
        { "DamageMin", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, DamageMin), sizeof(ITEM_ATTRIBUTE::DamageMin), -1 },
        { "DamageMax", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, DamageMax), sizeof(ITEM_ATTRIBUTE::DamageMax), -1 },
        { "SuccessfulBlocking", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, SuccessfulBlocking), sizeof(ITEM_ATTRIBUTE::SuccessfulBlocking), -1 },
        { "Defense", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Defense), sizeof(ITEM_ATTRIBUTE::Defense), -1 },
        { "MagicDefense", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, MagicDefense), sizeof(ITEM_ATTRIBUTE::MagicDefense), -1 },
        { "WeaponSpeed", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, WeaponSpeed), sizeof(ITEM_ATTRIBUTE::WeaponSpeed), -1 },
        { "WalkSpeed", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, WalkSpeed), sizeof(ITEM_ATTRIBUTE::WalkSpeed), -1 },
        { "Durability", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Durability), sizeof(ITEM_ATTRIBUTE::Durability), -1 },
        { "MagicDur", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, MagicDur), sizeof(ITEM_ATTRIBUTE::MagicDur), -1 },
        { "MagicPower", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, MagicPower), sizeof(ITEM_ATTRIBUTE::MagicPower), -1 },
        { "RequireStrength", FieldType::Word, offsetof(ITEM_ATTRIBUTE, RequireStrength), sizeof(ITEM_ATTRIBUTE::RequireStrength), -1 },
        { "RequireDexterity", FieldType::Word, offsetof(ITEM_ATTRIBUTE, RequireDexterity), sizeof(ITEM_ATTRIBUTE::RequireDexterity), -1 },
        { "RequireEnergy", FieldType::Word, offsetof(ITEM_ATTRIBUTE, RequireEnergy), sizeof(ITEM_ATTRIBUTE::RequireEnergy), -1 },
        { "RequireVitality", FieldType::Word, offsetof(ITEM_ATTRIBUTE, RequireVitality), sizeof(ITEM_ATTRIBUTE::RequireVitality), -1 },
        { "RequireCharisma", FieldType::Word, offsetof(ITEM_ATTRIBUTE, RequireCharisma), sizeof(ITEM_ATTRIBUTE::RequireCharisma), -1 },
        { "RequireLevel", FieldType::Word, offsetof(ITEM_ATTRIBUTE, RequireLevel), sizeof(ITEM_ATTRIBUTE::RequireLevel), -1 },
        { "Value", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Value), sizeof(ITEM_ATTRIBUTE::Value), -1 },
        { "iZen", FieldType::Int, offsetof(ITEM_ATTRIBUTE, iZen), sizeof(ITEM_ATTRIBUTE::iZen), -1 },
        { "AttType", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, AttType), sizeof(ITEM_ATTRIBUTE::AttType), -1 }
    };

    // Array fields (RequireClass[0-6], Resistance[0-7])
    inline constexpr FieldInfo ARRAY_FIELDS[] = {
        { "RequireClass[0]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 0, sizeof(BYTE), 0 },
        { "RequireClass[1]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 1, sizeof(BYTE), 1 },
        { "RequireClass[2]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 2, sizeof(BYTE), 2 },
        { "RequireClass[3]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 3, sizeof(BYTE), 3 },
        { "RequireClass[4]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 4, sizeof(BYTE), 4 },
        { "RequireClass[5]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 5, sizeof(BYTE), 5 },
        { "RequireClass[6]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 6, sizeof(BYTE), 6 },
        { "Resistance[0]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Resistance) + sizeof(BYTE) * 0, sizeof(BYTE), 0 },
        { "Resistance[1]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Resistance) + sizeof(BYTE) * 1, sizeof(BYTE), 1 },
        { "Resistance[2]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Resistance) + sizeof(BYTE) * 2, sizeof(BYTE), 2 },
        { "Resistance[3]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Resistance) + sizeof(BYTE) * 3, sizeof(BYTE), 3 },
        { "Resistance[4]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Resistance) + sizeof(BYTE) * 4, sizeof(BYTE), 4 },
        { "Resistance[5]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Resistance) + sizeof(BYTE) * 5, sizeof(BYTE), 5 },
        { "Resistance[6]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Resistance) + sizeof(BYTE) * 6, sizeof(BYTE), 6 },
        { "Resistance[7]", FieldType::Byte, offsetof(ITEM_ATTRIBUTE, Resistance) + sizeof(BYTE) * 7, sizeof(BYTE), 7 }
    };

    // Compile-time field counts
    inline constexpr size_t SIMPLE_FIELD_COUNT = std::size(SIMPLE_FIELDS);
    inline constexpr size_t ARRAY_FIELD_COUNT = std::size(ARRAY_FIELDS);
    inline constexpr size_t TOTAL_FIELD_COUNT = SIMPLE_FIELD_COUNT + ARRAY_FIELD_COUNT;
}

#endif // _EDITOR
