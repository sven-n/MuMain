#include "stdafx.h"

#include "DataHandler/SkillData/SkillDataLoader.h"
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
#ifdef _EDITOR
        g_MuEditorConsoleUI.LogEditor(StringUtils::WideToNarrow(errorMsg));
#else
        MessageBox(g_hWnd, errorMsg, NULL, MB_OK);
#endif
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
    int readSize = isLegacyFormat ? LegacySize : NewSize;

    // Validate file size
    if (fileSize != expectedLegacySize && fileSize != expectedNewSize)
    {
        fclose(fp);
        wchar_t errorMsg[256];
        swprintf(errorMsg, L"Skill file size mismatch. Expected: %ld (new) or %ld (legacy), Got: %ld",
                 expectedNewSize, expectedLegacySize, fileSize);
#ifdef _EDITOR
        g_MuEditorConsoleUI.LogEditor(StringUtils::WideToNarrow(errorMsg));
#else
        MessageBox(g_hWnd, errorMsg, NULL, MB_OK);
#endif
        return false;
    }

#ifdef _EDITOR
    if (isLegacyFormat)
    {
        g_MuEditorConsoleUI.LogEditor("Detected legacy skill format (32-byte names)");
    }
#endif

    // Read file data
    BYTE* Buffer = new BYTE[readSize * MAX_SKILLS];
    fread(Buffer, readSize * MAX_SKILLS, 1, fp);

    // Read checksum
    DWORD dwCheckSum;
    fread(&dwCheckSum, sizeof(DWORD), 1, fp);
    fclose(fp);

    // Verify checksum
    DWORD calculatedChecksum = GenerateCheckSum2(Buffer, readSize * MAX_SKILLS, 0x5A18);
    if (dwCheckSum != calculatedChecksum)
    {
        delete[] Buffer;
        wchar_t errorMsg[256];
        swprintf(errorMsg, L"Skill file corrupted: %ls (checksum mismatch)", fileName);
#ifdef _EDITOR
        g_MuEditorConsoleUI.LogEditor(StringUtils::WideToNarrow(errorMsg));
#else
        MessageBox(g_hWnd, errorMsg, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
#endif
        return false;
    }

    // Decrypt and parse data
    BYTE* pSeek = Buffer;
    for (int i = 0; i < MAX_SKILLS; i++)
    {
        BuxConvert(pSeek, readSize);

        if (isLegacyFormat)
        {
            SKILL_ATTRIBUTE_FILE_LEGACY source;
            memcpy(&source, pSeek, LegacySize);
            CopySkillAttributeFromSource(SkillAttribute[i], source);
        }
        else
        {
            SKILL_ATTRIBUTE_FILE source;
            memcpy(&source, pSeek, NewSize);
            CopySkillAttributeFromSource(SkillAttribute[i], source);
        }

        pSeek += readSize;
    }

    delete[] Buffer;

#ifdef _EDITOR
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
#endif

    return true;
}
