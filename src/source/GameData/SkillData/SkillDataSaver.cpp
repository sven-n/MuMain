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
#include <sstream>
#include <string>

// Helper to convert wide string to narrow string
static std::string WideToNarrow(const wchar_t* wstr)
{
    if (!wstr) return "";
    size_t len = wcslen(wstr);
    if (len == 0) return "";

    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, (int)len, NULL, 0, NULL, NULL);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, (int)len, &result[0], size, NULL, NULL);
    return result;
}

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
            BYTE* OrigBuffer = new BYTE[Size * MAX_SKILLS];
            fread(OrigBuffer, Size * MAX_SKILLS, 1, fpOrig);
            DWORD dwOrigCheckSum;
            fread(&dwOrigCheckSum, sizeof(DWORD), 1, fpOrig);
            fclose(fpOrig);

            // Decrypt original data
            BYTE* pSeek = OrigBuffer;
            for (int i = 0; i < MAX_SKILLS; i++)
            {
                BuxConvert(pSeek, Size);

                char rawName[MAX_SKILL_NAME]{};
                memcpy(rawName, pSeek, MAX_SKILL_NAME);
                CMultiLanguage::ConvertFromUtf8(originalSkills[i].Name, rawName);
                pSeek += MAX_SKILL_NAME;

                memcpy(&(originalSkills[i].Level), pSeek, Size - MAX_SKILL_NAME);
                pSeek += Size - MAX_SKILL_NAME;
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

        // Convert name from UTF-16 to UTF-8
        CMultiLanguage::ConvertToUtf8(dest.Name, SkillAttribute[i].Name, MAX_SKILL_NAME);

        // Copy all other fields
        dest.Level = SkillAttribute[i].Level;
        dest.Damage = SkillAttribute[i].Damage;
        dest.Mana = SkillAttribute[i].Mana;
        dest.AbilityGuage = SkillAttribute[i].AbilityGuage;
        dest.Distance = SkillAttribute[i].Distance;
        dest.Delay = SkillAttribute[i].Delay;
        dest.Energy = SkillAttribute[i].Energy;
        dest.Charisma = SkillAttribute[i].Charisma;
        dest.MasteryType = SkillAttribute[i].MasteryType;
        dest.SkillUseType = SkillAttribute[i].SkillUseType;
        dest.SkillBrand = SkillAttribute[i].SkillBrand;
        dest.KillCount = SkillAttribute[i].KillCount;
        memcpy(dest.RequireDutyClass, SkillAttribute[i].RequireDutyClass, MAX_DUTY_CLASS);
        memcpy(dest.RequireClass, SkillAttribute[i].RequireClass, MAX_CLASS);
        dest.SkillRank = SkillAttribute[i].SkillRank;
        dest.Magic_Icon = SkillAttribute[i].Magic_Icon;
        dest.TypeSkill = SkillAttribute[i].TypeSkill;
        dest.Strength = SkillAttribute[i].Strength;
        dest.Dexterity = SkillAttribute[i].Dexterity;
        dest.ItemSkill = SkillAttribute[i].ItemSkill;
        dest.IsDamage = SkillAttribute[i].IsDamage;
        dest.Effect = SkillAttribute[i].Effect;

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
