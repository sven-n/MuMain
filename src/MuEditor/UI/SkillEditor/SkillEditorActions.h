#pragma once

#ifdef _EDITOR

#include "GameData/SkillData/SkillFieldMetadata.h"
#include "_struct.h"
#include <string>

// Handles Save/Export action buttons for the Skill Editor
// Uses metadata-driven approach for automatic export generation
class CSkillEditorActions
{
public:
    static void RenderSaveButton();
    static void RenderExportCSVButton();

    // Render all action buttons in a row
    static void RenderAllButtons();

    // Export skill data to CSV format (metadata-driven)
    static std::string ExportSkillToCSV(int skillIndex, SKILL_ATTRIBUTE& skill);

    // Export skill data to readable format (key=value pairs, metadata-driven)
    static std::string ExportSkillToReadable(int skillIndex, SKILL_ATTRIBUTE& skill);

    // Export both formats combined (readable + CSV)
    static std::string ExportSkillCombined(int skillIndex, SKILL_ATTRIBUTE& skill);

    // Get CSV header row (metadata-driven)
    static std::string GetCSVHeader();

private:
    // Helper to convert skill name to UTF-8
    static void ConvertSkillName(char* outBuffer, size_t bufferSize, const wchar_t* name);

    // Get field value as string (descriptor-driven)
    static std::string GetFieldValueAsString(const SKILL_ATTRIBUTE& skill, const SkillFieldDescriptor& desc);
};

#endif // _EDITOR
