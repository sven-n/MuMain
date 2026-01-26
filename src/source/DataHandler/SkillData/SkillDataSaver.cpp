#include "stdafx.h"

#ifdef _EDITOR

#include "SkillDataSaver.h"
#include "GameData/SkillData/SkillStructs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Utilities/StringUtils.h"
#include <sstream>
#include <string>
#include <memory>

#include "DataHandler/CommonDataSaver.h"
#include "DataHandler/FieldMetadata.h"
#include "DataHandler/SkillData/SkillComparisonMetadata.h"

// External references
extern SKILL_ATTRIBUTE* SkillAttribute;

// Comparison function - uses field metadata (NO MACROS!)
static void CompareSkills(const SKILL_ATTRIBUTE& oldSkill, const SKILL_ATTRIBUTE& newSkill,
                         std::stringstream& changes, bool& changed)
{
    // Compare Name field (wide string)
    if (wcscmp(oldSkill.Name, newSkill.Name) != 0)
    {
        char oldUtf8[256];
        char newUtf8[256];
        WideCharToMultiByte(CP_UTF8, 0, oldSkill.Name, -1, oldUtf8, sizeof(oldUtf8), NULL, NULL);
        WideCharToMultiByte(CP_UTF8, 0, newSkill.Name, -1, newUtf8, sizeof(newUtf8), NULL, NULL);

        changes << "  Name: \"" << oldUtf8 << "\" -> \"" << newUtf8 << "\"\n";
        changed = true;
    }

    // Compare fields before arrays - simple loop!
    CompareAllFieldsByMetadata(oldSkill, newSkill,
                               SkillComparisonMetadata::FIELDS_BEFORE_ARRAYS,
                               changes, changed);

    // Compare array fields - another simple loop!
    CompareAllFieldsByMetadata(oldSkill, newSkill,
                               SkillComparisonMetadata::ARRAY_FIELDS,
                               changes, changed);

    // Compare fields after arrays - one more simple loop!
    CompareAllFieldsByMetadata(oldSkill, newSkill,
                               SkillComparisonMetadata::FIELDS_AFTER_ARRAYS,
                               changes, changed);
}

bool SkillDataSaver::Save(wchar_t* fileName, std::string* outChangeLog)
{
    // Create standard save config with skill-specific parameters
    auto config = CreateStandardSaveConfig<SKILL_ATTRIBUTE, SKILL_ATTRIBUTE_FILE>(
        fileName,
        MAX_SKILLS,
        SkillAttribute,
        [](SKILL_ATTRIBUTE_FILE& dest, SKILL_ATTRIBUTE& src) {
            CopySkillAttributeToDestination(dest, src);
        },
        [](SKILL_ATTRIBUTE& dest, SKILL_ATTRIBUTE_FILE& src) {
            CopySkillAttributeFromSource(dest, src);
        },
        CompareSkills,
        [](int index, const SKILL_ATTRIBUTE& skill) {
            return ChangeTracker::GetNameUtf8(index, skill, MAX_SKILL_NAME);
        },
        0x5A18,  // Skill-specific checksum key
        outChangeLog
    );

    // Add legacy format support for backwards compatibility with old Skill.bmd files
    config.legacyFileStructSize = sizeof(SKILL_ATTRIBUTE_FILE_LEGACY);
    config.convertFromFileLegacy = [](SKILL_ATTRIBUTE& dest, BYTE* buffer, size_t size) {
        SKILL_ATTRIBUTE_FILE_LEGACY legacyStruct;
        memcpy(&legacyStruct, buffer, sizeof(legacyStruct));
        CopySkillAttributeFromSource(dest, legacyStruct);
    };

    // Use generic save method
    bool result = CommonDataSaver::SaveData(config);

    // Adjust change log header
    if (result && outChangeLog && !outChangeLog->empty())
    {
        size_t pos = outChangeLog->find("=== Changes");
        if (pos != std::string::npos)
        {
            outChangeLog->replace(pos, 11, "=== Skill Changes");
        }
    }

    return result;
}

#endif // _EDITOR
