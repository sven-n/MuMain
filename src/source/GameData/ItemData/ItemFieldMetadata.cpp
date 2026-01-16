#include "stdafx.h"

#ifdef _EDITOR

#include "ItemFieldMetadata.h"
#include "ItemStructs.h"
#include "Translation/i18n.h"

std::vector<ItemFieldMetadata> CItemFieldMetadataRegistry::s_metadata;
bool CItemFieldMetadataRegistry::s_initialized = false;

const char* ItemFieldMetadata::GetDisplayName() const
{
    // Build translation key: "field_" + fieldName
    std::string translationKey = std::string("field_") + fieldName;

    // Try to get translation from metadata.json
    if (i18n::HasTranslation(i18n::Domain::Metadata, translationKey.c_str()))
    {
        return i18n::TranslateMetadata(translationKey.c_str(), defaultDisplayName);
    }

    // Fall back to default display name
    return defaultDisplayName;
}

void CItemFieldMetadataRegistry::InitializeMetadata()
{
    if (s_initialized) return;

    // Clear any existing metadata
    s_metadata.clear();

    // Define metadata for each field in ITEM_ATTRIBUTE
    // Format: (fieldName, displayName, offset, type, arraySize, columnWidth)

    s_metadata.push_back(ItemFieldMetadata("Name", "Name", FIELD_OFFSET(ITEM_ATTRIBUTE, Name), EItemFieldType::WCharArray, MAX_ITEM_NAME, 150.0f));
    s_metadata.push_back(ItemFieldMetadata("TwoHand", "Two-Hand", FIELD_OFFSET(ITEM_ATTRIBUTE, TwoHand), EItemFieldType::Bool, 1, 70.0f));
    s_metadata.push_back(ItemFieldMetadata("Level", "Level", FIELD_OFFSET(ITEM_ATTRIBUTE, Level), EItemFieldType::Word, 1, 60.0f));
    s_metadata.push_back(ItemFieldMetadata("m_byItemSlot", "Slot", FIELD_OFFSET(ITEM_ATTRIBUTE, m_byItemSlot), EItemFieldType::Byte, 1, 60.0f));
    s_metadata.push_back(ItemFieldMetadata("m_wSkillIndex", "Skill", FIELD_OFFSET(ITEM_ATTRIBUTE, m_wSkillIndex), EItemFieldType::Word, 1, 60.0f));
    s_metadata.push_back(ItemFieldMetadata("Width", "Width", FIELD_OFFSET(ITEM_ATTRIBUTE, Width), EItemFieldType::Byte, 1, 60.0f));
    s_metadata.push_back(ItemFieldMetadata("Height", "Height", FIELD_OFFSET(ITEM_ATTRIBUTE, Height), EItemFieldType::Byte, 1, 60.0f));
    s_metadata.push_back(ItemFieldMetadata("DamageMin", "Min Damage", FIELD_OFFSET(ITEM_ATTRIBUTE, DamageMin), EItemFieldType::Byte, 1, 70.0f));
    s_metadata.push_back(ItemFieldMetadata("DamageMax", "Max Damage", FIELD_OFFSET(ITEM_ATTRIBUTE, DamageMax), EItemFieldType::Byte, 1, 70.0f));
    s_metadata.push_back(ItemFieldMetadata("SuccessfulBlocking", "Block Rate", FIELD_OFFSET(ITEM_ATTRIBUTE, SuccessfulBlocking), EItemFieldType::Byte, 1, 70.0f));
    s_metadata.push_back(ItemFieldMetadata("Defense", "Defense", FIELD_OFFSET(ITEM_ATTRIBUTE, Defense), EItemFieldType::Byte, 1, 70.0f));
    s_metadata.push_back(ItemFieldMetadata("MagicDefense", "Magic Defense", FIELD_OFFSET(ITEM_ATTRIBUTE, MagicDefense), EItemFieldType::Byte, 1, 80.0f));
    s_metadata.push_back(ItemFieldMetadata("WeaponSpeed", "Attack Speed", FIELD_OFFSET(ITEM_ATTRIBUTE, WeaponSpeed), EItemFieldType::Byte, 1, 80.0f));
    s_metadata.push_back(ItemFieldMetadata("WalkSpeed", "Move Speed", FIELD_OFFSET(ITEM_ATTRIBUTE, WalkSpeed), EItemFieldType::Byte, 1, 80.0f));
    s_metadata.push_back(ItemFieldMetadata("Durability", "Durability", FIELD_OFFSET(ITEM_ATTRIBUTE, Durability), EItemFieldType::Byte, 1, 70.0f));
    s_metadata.push_back(ItemFieldMetadata("MagicDur", "Magic Durability", FIELD_OFFSET(ITEM_ATTRIBUTE, MagicDur), EItemFieldType::Byte, 1, 90.0f));
    s_metadata.push_back(ItemFieldMetadata("MagicPower", "Magic Power", FIELD_OFFSET(ITEM_ATTRIBUTE, MagicPower), EItemFieldType::Byte, 1, 80.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireStrength", "Req Strength", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireStrength), EItemFieldType::Word, 1, 80.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireDexterity", "Req Dexterity", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireDexterity), EItemFieldType::Word, 1, 80.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireEnergy", "Req Energy", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireEnergy), EItemFieldType::Word, 1, 80.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireVitality", "Req Vitality", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireVitality), EItemFieldType::Word, 1, 80.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireCharisma", "Req Leadership", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireCharisma), EItemFieldType::Word, 1, 90.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireLevel", "Req Level", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireLevel), EItemFieldType::Word, 1, 70.0f));
    s_metadata.push_back(ItemFieldMetadata("Value", "Sell Value", FIELD_OFFSET(ITEM_ATTRIBUTE, Value), EItemFieldType::Byte, 1, 70.0f));
    s_metadata.push_back(ItemFieldMetadata("iZen", "Buy Price", FIELD_OFFSET(ITEM_ATTRIBUTE, iZen), EItemFieldType::Int, 1, 80.0f));
    s_metadata.push_back(ItemFieldMetadata("AttType", "Attack Type", FIELD_OFFSET(ITEM_ATTRIBUTE, AttType), EItemFieldType::Byte, 1, 80.0f));

    // Array fields - RequireClass (individual elements)
    s_metadata.push_back(ItemFieldMetadata("RequireClass[0]", "DW/SM", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireClass[0]), EItemFieldType::Byte, 1, 60.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireClass[1]", "DK/BK", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireClass[1]), EItemFieldType::Byte, 1, 60.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireClass[2]", "ELF/ME", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireClass[2]), EItemFieldType::Byte, 1, 65.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireClass[3]", "MG/DM", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireClass[3]), EItemFieldType::Byte, 1, 60.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireClass[4]", "DL/LE", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireClass[4]), EItemFieldType::Byte, 1, 60.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireClass[5]", "SUM/BS", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireClass[5]), EItemFieldType::Byte, 1, 65.0f));
    s_metadata.push_back(ItemFieldMetadata("RequireClass[6]", "RF/FM", FIELD_OFFSET(ITEM_ATTRIBUTE, RequireClass[6]), EItemFieldType::Byte, 1, 60.0f));

    // Array fields - Resistance (individual elements)
    s_metadata.push_back(ItemFieldMetadata("Resistance[0]", "Ice Res", FIELD_OFFSET(ITEM_ATTRIBUTE, Resistance[0]), EItemFieldType::Byte, 1, 65.0f));
    s_metadata.push_back(ItemFieldMetadata("Resistance[1]", "Poison Res", FIELD_OFFSET(ITEM_ATTRIBUTE, Resistance[1]), EItemFieldType::Byte, 1, 75.0f));
    s_metadata.push_back(ItemFieldMetadata("Resistance[2]", "Lightning Res", FIELD_OFFSET(ITEM_ATTRIBUTE, Resistance[2]), EItemFieldType::Byte, 1, 85.0f));
    s_metadata.push_back(ItemFieldMetadata("Resistance[3]", "Fire Res", FIELD_OFFSET(ITEM_ATTRIBUTE, Resistance[3]), EItemFieldType::Byte, 1, 65.0f));
    s_metadata.push_back(ItemFieldMetadata("Resistance[4]", "Earth Res", FIELD_OFFSET(ITEM_ATTRIBUTE, Resistance[4]), EItemFieldType::Byte, 1, 70.0f));
    s_metadata.push_back(ItemFieldMetadata("Resistance[5]", "Wind Res", FIELD_OFFSET(ITEM_ATTRIBUTE, Resistance[5]), EItemFieldType::Byte, 1, 70.0f));
    s_metadata.push_back(ItemFieldMetadata("Resistance[6]", "Water Res", FIELD_OFFSET(ITEM_ATTRIBUTE, Resistance[6]), EItemFieldType::Byte, 1, 75.0f));
    s_metadata.push_back(ItemFieldMetadata("Resistance[7]", "Res 7", FIELD_OFFSET(ITEM_ATTRIBUTE, Resistance[7]), EItemFieldType::Byte, 1, 60.0f));

    s_initialized = true;
}

const std::vector<ItemFieldMetadata>& CItemFieldMetadataRegistry::GetAllFields()
{
    InitializeMetadata();
    return s_metadata;
}

const ItemFieldMetadata* CItemFieldMetadataRegistry::FindFieldByName(const char* fieldName)
{
    InitializeMetadata();
    for (const auto& meta : s_metadata)
    {
        if (strcmp(meta.fieldName, fieldName) == 0)
            return &meta;
    }
    return nullptr;
}

int CItemFieldMetadataRegistry::GetFieldCount()
{
    InitializeMetadata();
    return (int)s_metadata.size();
}

#endif // _EDITOR
