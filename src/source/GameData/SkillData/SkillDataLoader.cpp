#include "stdafx.h"

#include "../../../../Source Main 5.2/source/DataHandler/SkillData/SkillDataLoader.h"
#include "GameData/SkillData/SkillStructs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"

#ifdef _EDITOR
#include "MuEditor/UI/Console/MuEditorConsoleUI.h"
#include <string>

// Helper to convert wide string to narrow string for logging
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
        g_MuEditorConsoleUI.LogEditor(WideToNarrow(errorMsg));
#else
        MessageBox(g_hWnd, errorMsg, NULL, MB_OK);
#endif
        return false;
    }

    const int Size = sizeof(SKILL_ATTRIBUTE_FILE);
    const long expectedSize = Size * MAX_SKILLS + sizeof(DWORD);

    // Verify file size
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fileSize != expectedSize)
    {
        fclose(fp);
        wchar_t errorMsg[256];
        swprintf(errorMsg, L"Skill file size mismatch. Expected: %ld, Got: %ld", expectedSize, fileSize);
#ifdef _EDITOR
        g_MuEditorConsoleUI.LogEditor(WideToNarrow(errorMsg));
#else
        MessageBox(g_hWnd, errorMsg, NULL, MB_OK);
#endif
        return false;
    }

    // Read file data
    BYTE* Buffer = new BYTE[Size * MAX_SKILLS];
    fread(Buffer, Size * MAX_SKILLS, 1, fp);

    // Read checksum
    DWORD dwCheckSum;
    fread(&dwCheckSum, sizeof(DWORD), 1, fp);
    fclose(fp);

    // Verify checksum
    DWORD calculatedChecksum = GenerateCheckSum2(Buffer, Size * MAX_SKILLS, 0x5A18);
    if (dwCheckSum != calculatedChecksum)
    {
        delete[] Buffer;
        wchar_t errorMsg[256];
        swprintf(errorMsg, L"Skill file corrupted: %ls (checksum mismatch)", fileName);
#ifdef _EDITOR
        g_MuEditorConsoleUI.LogEditor(WideToNarrow(errorMsg));
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
        BuxConvert(pSeek, Size);

        // Read name as UTF-8, convert to UTF-16
        char rawName[MAX_SKILL_NAME]{};
        memcpy(rawName, pSeek, MAX_SKILL_NAME);
        CMultiLanguage::ConvertFromUtf8(SkillAttribute[i].Name, rawName);
        pSeek += MAX_SKILL_NAME;

        // Copy remaining fields
        memcpy(&(SkillAttribute[i].Level), pSeek, Size - MAX_SKILL_NAME);
        pSeek += Size - MAX_SKILL_NAME;
    }

    delete[] Buffer;

#ifdef _EDITOR
    wchar_t successMsg[256];
    swprintf(successMsg, L"Loaded %d skills from %ls", MAX_SKILLS, fileName);
    g_MuEditorConsoleUI.LogEditor(WideToNarrow(successMsg));
#endif

    return true;
}
