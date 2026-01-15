#include "stdafx.h"

#ifdef _EDITOR

#include "SkillDataSaver.h"
#include "GameData/SkillData/SkillStructs.h"
#include "GameData/SkillData/SkillFieldDefs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"
#include "MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Utilities/StringUtils.h"
#include <sstream>
#include <string>

// External references
extern SKILL_ATTRIBUTE* SkillAttribute;

// X-Macro helpers for change tracking
#define COMPARE_FIELD_Byte(name, type, arraySize, width) \
    if (oldSkill.name != newSkill.name) { \
        skillChanges << "  " #name ": " << (int)oldSkill.name << " -> " << (int)newSkill.name << "\n"; \
        changed = true; \
    }

#define COMPARE_FIELD_Word(name, type, arraySize, width) \
    if (oldSkill.name != newSkill.name) { \
        skillChanges << "  " #name ": " << oldSkill.name << " -> " << newSkill.name << "\n"; \
        changed = true; \
    }

#define COMPARE_FIELD_Int(name, type, arraySize, width) \
    if (oldSkill.name != newSkill.name) { \
        skillChanges << "  " #name ": " << oldSkill.name << " -> " << newSkill.name << "\n"; \
        changed = true; \
    }

#define COMPARE_FIELD_DWord(name, type, arraySize, width) \
    if (oldSkill.name != newSkill.name) { \
        skillChanges << "  " #name ": " << oldSkill.name << " -> " << newSkill.name << "\n"; \
        changed = true; \
    }

#define COMPARE_ARRAY_FIELD(nameWithIndex, baseName, index, type, width) \
    if (oldSkill.baseName[index] != newSkill.baseName[index]) { \
        skillChanges << "  " #nameWithIndex ": " << (int)oldSkill.baseName[index] << " -> " << (int)newSkill.baseName[index] << "\n"; \
        changed = true; \
    }

bool SkillDataSaver::CreateBackup(wchar_t* fileName)
{
    wchar_t backupName[512];
    swprintf(backupName, L"%ls.bak", fileName);

    // Delete old backup if exists
    DeleteFileW(backupName);

    // Copy current file to backup
    if (!CopyFileW(fileName, backupName, FALSE))
    {
        g_MuEditorConsoleUI.LogEditor("Warning: Could not create backup file");
        return false;
    }

    return true;
}

bool SkillDataSaver::Save(wchar_t* fileName, std::string* outChangeLog)
{
    const int Size = sizeof(SKILL_ATTRIBUTE_FILE);

    // Load original file for comparison if change log is requested
    SKILL_ATTRIBUTE* originalSkills = nullptr;
    if (outChangeLog)
    {
        originalSkills = new SKILL_ATTRIBUTE[MAX_SKILLS];
        memset(originalSkills, 0, sizeof(SKILL_ATTRIBUTE) * MAX_SKILLS);

        FILE* fpOrig = _wfopen(fileName, L"rb");
        if (fpOrig)
        {
            // Get file size to determine structure version
            fseek(fpOrig, 0, SEEK_END);
            long fileSize = ftell(fpOrig);
            fseek(fpOrig, 0, SEEK_SET);

            const int LegacySize = sizeof(SKILL_ATTRIBUTE_FILE_LEGACY);
            const int NewSize = sizeof(SKILL_ATTRIBUTE_FILE);
            const long expectedLegacySize = LegacySize * MAX_SKILLS + sizeof(DWORD);
            const long expectedNewSize = NewSize * MAX_SKILLS + sizeof(DWORD);

            bool isLegacyFormat = (fileSize == expectedLegacySize);
            int readSize = isLegacyFormat ? LegacySize : NewSize;

            BYTE* OrigBuffer = new BYTE[readSize * MAX_SKILLS];
            fread(OrigBuffer, readSize * MAX_SKILLS, 1, fpOrig);
            DWORD dwOrigCheckSum;
            fread(&dwOrigCheckSum, sizeof(DWORD), 1, fpOrig);
            fclose(fpOrig);

            // Decrypt original data
            BYTE* pSeek = OrigBuffer;
            for (int i = 0; i < MAX_SKILLS; i++)
            {
                BuxConvert(pSeek, readSize);

                if (isLegacyFormat)
                {
                    SKILL_ATTRIBUTE_FILE_LEGACY source;
                    memcpy(&source, pSeek, LegacySize);
                    CopySkillAttributeFromSource(originalSkills[i], source);
                }
                else
                {
                    SKILL_ATTRIBUTE_FILE source;
                    memcpy(&source, pSeek, NewSize);
                    CopySkillAttributeFromSource(originalSkills[i], source);
                }

                pSeek += readSize;
            }
            delete[] OrigBuffer;
        }
    }

    // Create backup
    CreateBackup(fileName);

    // Prepare new buffer
    BYTE* Buffer = new BYTE[Size * MAX_SKILLS];
    BYTE* pSeek = Buffer;

    // Track changes
    std::stringstream changeLog;
    int changeCount = 0;

    // Convert SKILL_ATTRIBUTE to SKILL_ATTRIBUTE_FILE format
    for (int i = 0; i < MAX_SKILLS; i++)
    {
        SKILL_ATTRIBUTE_FILE dest;
        memset(&dest, 0, Size);

        CopySkillAttributeToDestination(dest, SkillAttribute[i]);

        // Track changes using X-macros
        if (originalSkills && SkillAttribute[i].Name[0] != 0)
        {
            bool changed = false;
            std::stringstream skillChanges;

            SKILL_ATTRIBUTE& oldSkill = originalSkills[i];
            SKILL_ATTRIBUTE& newSkill = SkillAttribute[i];

            // Name (special case - wide char comparison)
            if (wcscmp(oldSkill.Name, newSkill.Name) != 0)
            {
                char oldName[MAX_SKILL_NAME];
                char newName[MAX_SKILL_NAME];
                CMultiLanguage::ConvertToUtf8(oldName, oldSkill.Name, MAX_SKILL_NAME);
                CMultiLanguage::ConvertToUtf8(newName, newSkill.Name, MAX_SKILL_NAME);
                skillChanges << "  Name: \"" << oldName << "\" -> \"" << newName << "\"\n";
                changed = true;
            }

            // All other fields using X-macros
            #define COMPARE_SIMPLE(name, type, arraySize, width) COMPARE_FIELD_##type(name, type, arraySize, width)
            SKILL_FIELDS_SIMPLE(COMPARE_SIMPLE)
            #undef COMPARE_SIMPLE

            SKILL_FIELDS_ARRAYS(COMPARE_ARRAY_FIELD)

            #define COMPARE_SIMPLE(name, type, arraySize, width) COMPARE_FIELD_##type(name, type, arraySize, width)
            SKILL_FIELDS_AFTER_ARRAYS(COMPARE_SIMPLE)
            #undef COMPARE_SIMPLE

            if (changed)
            {
                changeLog << "Skill " << i << ":\n" << skillChanges.str();
                changeCount++;
            }
        }

        // Encrypt and write to buffer
        BuxConvert((BYTE*)&dest, Size);
        memcpy(pSeek, &dest, Size);
        pSeek += Size;
    }

    // Check if we should skip saving when no changes detected
    if (originalSkills && changeCount == 0)
    {
        // No changes - skip save
        delete[] Buffer;
        delete[] originalSkills;

        if (outChangeLog)
        {
            *outChangeLog = "No changes detected.\n";
        }

        return false; // Return false to indicate no save was needed
    }

    // Generate checksum
    DWORD dwCheckSum = GenerateCheckSum2(Buffer, Size * MAX_SKILLS, 0x5A18);

    // Open file for writing only after we know we need to save
    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == NULL)
    {
        delete[] Buffer;
        if (originalSkills) delete[] originalSkills;
        return false;
    }

    fwrite(Buffer, Size * MAX_SKILLS, 1, fp);
    fwrite(&dwCheckSum, sizeof(DWORD), 1, fp);
    fclose(fp);

    delete[] Buffer;

    if (originalSkills)
    {
        delete[] originalSkills;
        if (outChangeLog && changeCount > 0)
        {
            *outChangeLog = "=== Skill Changes (" + std::to_string(changeCount) + " skills modified) ===\n" + changeLog.str();
        }
    }

    return true;
}

#endif // _EDITOR
