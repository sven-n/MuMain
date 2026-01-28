#include "stdafx.h"

#include "DataHandler/SkillData/SkillDataLoader.h"
#include "DataHandler/DataFileIO.h"
#include "GameData/SkillData/SkillStructs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"

#ifdef _EDITOR
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Utilities/StringUtils.h"
#include <string>
#endif

// External references
extern SKILL_ATTRIBUTE* SkillAttribute;
extern HWND g_hWnd;

bool SkillDataLoader::Load(wchar_t* fileName)
{
    FILE* fp = _wfopen(fileName, L"rb");
    if (fp == NULL)
    {
        wchar_t errorMsg[256];
        swprintf(errorMsg, L"Skill file not found: %ls", fileName);
        DataFileIO::ShowErrorAndExit(errorMsg);
        return false;
    }

    // Get file size to determine structure version
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    const int LegacySize = sizeof(SKILL_ATTRIBUTE_FILE_LEGACY);
    const int NewSize = sizeof(SKILL_ATTRIBUTE_FILE);
    const long expectedLegacySize = LegacySize * MAX_SKILLS + sizeof(DWORD);
    const long expectedNewSize = NewSize * MAX_SKILLS + sizeof(DWORD);

    bool isLegacyFormat = (fileSize == expectedLegacySize);
    bool success = false;

#ifdef _EDITOR
    if (isLegacyFormat)
    {
        g_MuEditorConsoleUI.LogEditor("Detected legacy skill format (32-byte names)");
    }
#endif

    if (isLegacyFormat)
    {
        success = LoadLegacyFormat(fp, fileSize);
    }
    else
    {
        success = LoadNewFormat(fp, fileSize);
    }

    fclose(fp);

#ifdef _EDITOR
    if (success)
    {
        // Count non-empty skills (skills with names)
        int skillCount = 0;
        for (int i = 0; i < MAX_SKILLS; i++)
        {
            if (SkillAttribute[i].Name[0] != L'\0')
            {
                skillCount++;
            }
        }

        wchar_t successMsg[256];
        swprintf(successMsg, L"Loaded %d skills from %ls", skillCount, fileName);
        g_MuEditorConsoleUI.LogEditor(StringUtils::WideToNarrow(successMsg));
    }
#endif

    return success;
}

template<typename TFileFormat>
bool SkillDataLoader::LoadFormat(FILE* fp, const wchar_t* formatName)
{
    const int Size = sizeof(TFileFormat);

    // Configure I/O
    DataFileIO::IOConfig config;
    config.itemSize = Size;
    config.itemCount = MAX_SKILLS;
    config.checksumKey = 0x5A18;
    config.decryptRecord = [](BYTE* data, int size) { BuxConvert(data, size); };

    // Read buffer and checksum
    DWORD dwCheckSum;
    auto buffer = DataFileIO::ReadBuffer(fp, config, &dwCheckSum);
    if (!buffer)
    {
        std::wstring errorMsg = L"Failed to read skill file (";
        errorMsg += formatName;
        errorMsg += L").";
        DataFileIO::ShowErrorAndExit(errorMsg.c_str());
        return false;
    }

    // Verify checksum
    if (!DataFileIO::VerifyChecksum(buffer.get(), config, dwCheckSum))
    {
        std::wstring errorMsg = L"Skill file corrupted (";
        errorMsg += formatName;
        errorMsg += L").";
        DataFileIO::ShowErrorAndExit(errorMsg.c_str());
        return false;
    }

    // Decrypt buffer
    DataFileIO::DecryptBuffer(buffer.get(), config);

    // Copy skills
    BYTE* pSeek = buffer.get();
    for (int i = 0; i < MAX_SKILLS; i++)
    {
        TFileFormat source;
        memcpy(&source, pSeek, sizeof(source));
        CopySkillAttributeFromSource(SkillAttribute[i], source);
        pSeek += Size;
    }

    return true;
}

bool SkillDataLoader::LoadLegacyFormat(FILE* fp, long fileSize)
{
    return LoadFormat<SKILL_ATTRIBUTE_FILE_LEGACY>(fp, L"legacy format");
}

bool SkillDataLoader::LoadNewFormat(FILE* fp, long fileSize)
{
    return LoadFormat<SKILL_ATTRIBUTE_FILE>(fp, L"new format");
}
