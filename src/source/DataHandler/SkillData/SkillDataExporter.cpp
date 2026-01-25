#include "stdafx.h"

#ifdef _EDITOR

#include "SkillDataExporter.h"
#include "GameData/SkillData/SkillStructs.h"
#include "GameData/SkillData/SkillFieldDefs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Utilities/StringUtils.h"
#include <string>

// External references
extern SKILL_ATTRIBUTE* SkillAttribute;

// X-Macro helpers for CSV generation
#define CSV_HEADER_SIMPLE(name, type, arraySize, width) "," #name
#define CSV_HEADER_ARRAY(nameWithIndex, baseName, index, type, width) "," #nameWithIndex

// Complete CSV header with Index and Name
#define CSV_FULL_HEADER "Index,Name" SKILL_FIELDS_SIMPLE(CSV_HEADER_SIMPLE) SKILL_FIELDS_ARRAYS(CSV_HEADER_ARRAY) SKILL_FIELDS_AFTER_ARRAYS(CSV_HEADER_SIMPLE)

// Helper to print field values
#define PRINT_FIELD_Byte(skill, name) fprintf(csvFp, ",%d", (skill).name)
#define PRINT_FIELD_Word(skill, name) fprintf(csvFp, ",%d", (skill).name)
#define PRINT_FIELD_Int(skill, name) fprintf(csvFp, ",%d", (skill).name)
#define PRINT_FIELD_DWord(skill, name) fprintf(csvFp, ",%u", (skill).name)

#define CSV_PRINT_SIMPLE(name, type, arraySize, width) PRINT_FIELD_##type(skill, name);
#define CSV_PRINT_ARRAY(nameWithIndex, baseName, index, type, width) fprintf(csvFp, ",%d", skill.baseName[index]);

bool SkillDataExporter::ExportToCsv(wchar_t* fileName)
{
    FILE* csvFp = _wfopen(fileName, L"w");
    if (!csvFp)
    {
        g_MuEditorConsoleUI.LogEditor("Failed to create CSV file for export");
        return false;
    }

    // Write BOM for UTF-8
    fprintf(csvFp, "\xEF\xBB\xBF");

    // Write CSV header using X-macros
    fprintf(csvFp, CSV_FULL_HEADER "\n");

    int exportedCount = 0;
    for (int i = 0; i < MAX_SKILLS; i++)
    {
        // Only export skills with names
        if (SkillAttribute[i].Name[0] != 0)
        {
            // Convert wide char name to UTF-8 using StringUtils
            std::string utf8Name = StringUtils::WideToNarrow(SkillAttribute[i].Name);

            // Escape quotes in name
            std::string escapedName;
            escapedName.reserve(utf8Name.length() * 2); // Reserve space for worst case
            for (char c : utf8Name)
            {
                if (c == '"')
                {
                    escapedName += "\"\"";
                }
                else
                {
                    escapedName += c;
                }
            }

            // Print index and name
            fprintf(csvFp, "%d,\"%s\"", i, escapedName.c_str());

            // Print all fields using X-macros
            SKILL_ATTRIBUTE& skill = SkillAttribute[i];
            SKILL_FIELDS_SIMPLE(CSV_PRINT_SIMPLE)
            SKILL_FIELDS_ARRAYS(CSV_PRINT_ARRAY)

            fprintf(csvFp, "\n");
            exportedCount++;
        }
    }

    fclose(csvFp);

    std::string successMsg = "Successfully exported " + std::to_string(exportedCount) + " skills to CSV";
    g_MuEditorConsoleUI.LogEditor(successMsg);

    return true;
}

#endif // _EDITOR
