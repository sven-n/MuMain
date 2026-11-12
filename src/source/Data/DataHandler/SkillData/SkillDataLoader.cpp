#include "stdafx.h"

#include "Data/DataHandler/SkillData/SkillDataLoader.h"
#include "Core/Content/ContentStore.h"
#include "Data/DataHandler/DataFileIO.h"
#include "Data/GameData/SkillData/SkillStructs.h"
#include "Core/Globals/_struct.h"
#include "Core/Globals/_define.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Data/Translation/MultiLanguage.h"

#ifdef _EDITOR
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Core/Utilities/StringUtils.h"
#include <string>
#endif

// External references
extern SKILL_ATTRIBUTE* SkillAttribute;
extern HWND g_hWnd;

bool SkillDataLoader::Load(wchar_t* fileName)
{
    const Content::Stream fp = Content::OpenIO(fileName);
    if (fp == nullptr)
    {
        wchar_t errorMsg[256];
        mu_swprintf(errorMsg, L"Skill file not found: %ls", fileName);
        DataFileIO::ReportError(errorMsg);
        return false;
    }

    // Get file size to determine structure version
    long fileSize = static_cast<long>(SDL_GetIOSize(fp.get()));

    const int LegacySize = sizeof(SKILL_ATTRIBUTE_FILE_LEGACY);
    const long expectedLegacySize = LegacySize * MAX_SKILLS + sizeof(DWORD);

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
        success = LoadLegacyFormat(fp.get(), fileSize);
    }
    else
    {
        success = LoadNewFormat(fp.get(), fileSize);
    }

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
        mu_swprintf(successMsg, L"Loaded %d skills from %ls", skillCount, fileName);
        g_MuEditorConsoleUI.LogEditor(StringUtils::WideToNarrow(successMsg));
    }
#endif

    return success;
}

template <typename TFileFormat> bool SkillDataLoader::LoadFormat(SDL_IOStream* fp, const wchar_t* formatName)
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
        DataFileIO::ReportError(errorMsg.c_str());
        return false;
    }

    // Verify checksum
    if (!DataFileIO::VerifyChecksum(buffer.get(), config, dwCheckSum))
    {
        std::wstring errorMsg = L"Skill file corrupted (";
        errorMsg += formatName;
        errorMsg += L").";
        DataFileIO::ReportError(errorMsg.c_str());
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

bool SkillDataLoader::LoadLegacyFormat(SDL_IOStream* fp, long fileSize)
{
    return LoadFormat<SKILL_ATTRIBUTE_FILE_LEGACY>(fp, L"legacy format");
}

bool SkillDataLoader::LoadNewFormat(SDL_IOStream* fp, long fileSize)
{
    return LoadFormat<SKILL_ATTRIBUTE_FILE>(fp, L"new format");
}
