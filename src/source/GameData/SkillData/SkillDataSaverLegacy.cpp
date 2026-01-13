#include "stdafx.h"

#ifdef _EDITOR

#include "SkillDataSaverLegacy.h"
#include "GameData/SkillData/SkillStructs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"

// External references
extern SKILL_ATTRIBUTE* SkillAttribute;

bool SkillDataSaverLegacy::SaveLegacy(wchar_t* fileName)
{
    const int Size = sizeof(SKILL_ATTRIBUTE_FILE_LEGACY);

    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == NULL)
    {
        return false;
    }

    BYTE* Buffer = new BYTE[Size * MAX_SKILLS];
    BYTE* pSeek = Buffer;

    // Convert SKILL_ATTRIBUTE to SKILL_ATTRIBUTE_FILE_LEGACY format
    for (int i = 0; i < MAX_SKILLS; i++)
    {
        SKILL_ATTRIBUTE_FILE_LEGACY dest;
        memset(&dest, 0, Size);

        CopySkillAttributeToDestination(dest, SkillAttribute[i]);

        // Encrypt and write to buffer
        BuxConvert((BYTE*)&dest, Size);
        memcpy(pSeek, &dest, Size);
        pSeek += Size;
    }

    // Generate checksum
    DWORD dwCheckSum = GenerateCheckSum2(Buffer, Size * MAX_SKILLS, 0x5A18);

    fwrite(Buffer, Size * MAX_SKILLS, 1, fp);
    fwrite(&dwCheckSum, sizeof(DWORD), 1, fp);
    fclose(fp);

    delete[] Buffer;

    return true;
}

#endif // _EDITOR
