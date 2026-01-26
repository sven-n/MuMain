#include "stdafx.h"

#ifdef _EDITOR

#include "SkillDataSaverLegacy.h"
#include "DataHandler/DataFileIO.h"
#include "GameData/SkillData/SkillStructs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include <memory>

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

    auto Buffer = std::make_unique<BYTE[]>(Size * MAX_SKILLS);
    BYTE* pSeek = Buffer.get();

    // Convert SKILL_ATTRIBUTE to SKILL_ATTRIBUTE_FILE_LEGACY format
    for (int i = 0; i < MAX_SKILLS; i++)
    {
        SKILL_ATTRIBUTE_FILE_LEGACY dest;
        memset(&dest, 0, Size);

        CopySkillAttributeToDestination(dest, SkillAttribute[i]);

        memcpy(pSeek, &dest, Size);
        pSeek += Size;
    }

    // Configure I/O
    DataFileIO::IOConfig config;
    config.itemSize = Size;
    config.itemCount = MAX_SKILLS;
    config.checksumKey = 0x5A18;
    config.encryptRecord = [](BYTE* data, int size) { BuxConvert(data, size); };

    // Encrypt buffer
    DataFileIO::EncryptBuffer(Buffer.get(), config);

    // Write buffer and checksum
    bool success = DataFileIO::WriteBuffer(fp, Buffer.get(), config);

    fclose(fp);

    return success;
}

#endif // _EDITOR
