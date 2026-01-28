#pragma once

#ifdef _EDITOR

#include "DataHandler/FieldMetadata.h"
#include "GameData/SkillData/SkillStructs.h"

/**
 * Skill field metadata for comparison - C++20 constexpr arrays
 *
 * ZERO MACROS! Crystal clear, explicit field definitions.
 * Every field is visible and easy to understand.
 */

namespace SkillComparisonMetadata
{
    // Simple fields (before arrays)
    inline constexpr FieldInfo FIELDS_BEFORE_ARRAYS[] = {
        { "Level", FieldType::Word, offsetof(SKILL_ATTRIBUTE, Level), sizeof(SKILL_ATTRIBUTE::Level), -1 },
        { "Damage", FieldType::Word, offsetof(SKILL_ATTRIBUTE, Damage), sizeof(SKILL_ATTRIBUTE::Damage), -1 },
        { "Mana", FieldType::Word, offsetof(SKILL_ATTRIBUTE, Mana), sizeof(SKILL_ATTRIBUTE::Mana), -1 },
        { "AbilityGuage", FieldType::Word, offsetof(SKILL_ATTRIBUTE, AbilityGuage), sizeof(SKILL_ATTRIBUTE::AbilityGuage), -1 },
        { "Distance", FieldType::DWord, offsetof(SKILL_ATTRIBUTE, Distance), sizeof(SKILL_ATTRIBUTE::Distance), -1 },
        { "Delay", FieldType::Int, offsetof(SKILL_ATTRIBUTE, Delay), sizeof(SKILL_ATTRIBUTE::Delay), -1 },
        { "Energy", FieldType::Int, offsetof(SKILL_ATTRIBUTE, Energy), sizeof(SKILL_ATTRIBUTE::Energy), -1 },
        { "Charisma", FieldType::Word, offsetof(SKILL_ATTRIBUTE, Charisma), sizeof(SKILL_ATTRIBUTE::Charisma), -1 },
        { "MasteryType", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, MasteryType), sizeof(SKILL_ATTRIBUTE::MasteryType), -1 },
        { "SkillUseType", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, SkillUseType), sizeof(SKILL_ATTRIBUTE::SkillUseType), -1 },
        { "SkillBrand", FieldType::DWord, offsetof(SKILL_ATTRIBUTE, SkillBrand), sizeof(SKILL_ATTRIBUTE::SkillBrand), -1 },
        { "KillCount", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, KillCount), sizeof(SKILL_ATTRIBUTE::KillCount), -1 }
    };

    // Array fields (RequireDutyClass[0-2], RequireClass[0-6])
    inline constexpr FieldInfo ARRAY_FIELDS[] = {
        { "RequireDutyClass[0]", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, RequireDutyClass) + sizeof(BYTE) * 0, sizeof(BYTE), 0 },
        { "RequireDutyClass[1]", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, RequireDutyClass) + sizeof(BYTE) * 1, sizeof(BYTE), 1 },
        { "RequireDutyClass[2]", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, RequireDutyClass) + sizeof(BYTE) * 2, sizeof(BYTE), 2 },
        { "RequireClass[0]", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 0, sizeof(BYTE), 0 },
        { "RequireClass[1]", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 1, sizeof(BYTE), 1 },
        { "RequireClass[2]", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 2, sizeof(BYTE), 2 },
        { "RequireClass[3]", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 3, sizeof(BYTE), 3 },
        { "RequireClass[4]", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 4, sizeof(BYTE), 4 },
        { "RequireClass[5]", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 5, sizeof(BYTE), 5 },
        { "RequireClass[6]", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, RequireClass) + sizeof(BYTE) * 6, sizeof(BYTE), 6 }
    };

    // Fields after arrays (must maintain binary layout order!)
    inline constexpr FieldInfo FIELDS_AFTER_ARRAYS[] = {
        { "SkillRank", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, SkillRank), sizeof(SKILL_ATTRIBUTE::SkillRank), -1 },
        { "Magic_Icon", FieldType::Word, offsetof(SKILL_ATTRIBUTE, Magic_Icon), sizeof(SKILL_ATTRIBUTE::Magic_Icon), -1 },
        { "TypeSkill", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, TypeSkill), sizeof(SKILL_ATTRIBUTE::TypeSkill), -1 },
        { "Strength", FieldType::Int, offsetof(SKILL_ATTRIBUTE, Strength), sizeof(SKILL_ATTRIBUTE::Strength), -1 },
        { "Dexterity", FieldType::Int, offsetof(SKILL_ATTRIBUTE, Dexterity), sizeof(SKILL_ATTRIBUTE::Dexterity), -1 },
        { "ItemSkill", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, ItemSkill), sizeof(SKILL_ATTRIBUTE::ItemSkill), -1 },
        { "IsDamage", FieldType::Byte, offsetof(SKILL_ATTRIBUTE, IsDamage), sizeof(SKILL_ATTRIBUTE::IsDamage), -1 },
        { "Effect", FieldType::Word, offsetof(SKILL_ATTRIBUTE, Effect), sizeof(SKILL_ATTRIBUTE::Effect), -1 }
    };

    // Compile-time field counts
    inline constexpr size_t BEFORE_ARRAY_FIELD_COUNT = std::size(FIELDS_BEFORE_ARRAYS);
    inline constexpr size_t ARRAY_FIELD_COUNT = std::size(ARRAY_FIELDS);
    inline constexpr size_t AFTER_ARRAY_FIELD_COUNT = std::size(FIELDS_AFTER_ARRAYS);
    inline constexpr size_t TOTAL_FIELD_COUNT = BEFORE_ARRAY_FIELD_COUNT + ARRAY_FIELD_COUNT + AFTER_ARRAY_FIELD_COUNT;
}

#endif // _EDITOR
