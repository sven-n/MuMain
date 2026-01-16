#include "stdafx.h"

#include "ItemDataLoader.h"
#include "ItemDataFileIO.h"
#include "GameData/ItemData/ItemAttributeHelpers.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"
#include "CSChaosCastle.h"

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

bool ItemDataLoader::Load(wchar_t* fileName)
{
    FILE* fp = _wfopen(fileName, L"rb");
    if (fp == NULL)
    {
        wchar_t Text[256];
        swprintf(Text, L"%ls - File not exist.", fileName);
        ItemDataFileIO::ShowErrorAndExit(Text);
        return false;
    }

    // Get file size to determine structure version
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    const int LegacySize = sizeof(ITEM_ATTRIBUTE_FILE_LEGACY);
    const int NewSize = sizeof(ITEM_ATTRIBUTE_FILE);
    const long expectedLegacySize = LegacySize * MAX_ITEM + sizeof(DWORD);
    const long expectedNewSize = NewSize * MAX_ITEM + sizeof(DWORD);

    bool isLegacyFormat = (fileSize == expectedLegacySize);
    bool success = false;

    if (isLegacyFormat)
    {
        success = LoadLegacyFormat(fp, fileSize);
    }
    else
    {
        success = LoadNewFormat(fp, fileSize);
    }

    fclose(fp);
    return success;
}

bool ItemDataLoader::LoadLegacyFormat(FILE* fp, long fileSize)
{
    const int Size = sizeof(ITEM_ATTRIBUTE_FILE_LEGACY);

    // Read buffer and checksum
    DWORD dwCheckSum;
    BYTE* Buffer = ItemDataFileIO::ReadAndDecryptBuffer(fp, Size, MAX_ITEM, &dwCheckSum);
    if (!Buffer)
    {
        ItemDataFileIO::ShowErrorAndExit(L"Failed to read item file (legacy format).");
        return false;
    }

    // Verify checksum
    if (!ItemDataFileIO::VerifyChecksum(Buffer, Size * MAX_ITEM, dwCheckSum))
    {
        delete[] Buffer;
        ItemDataFileIO::ShowErrorAndExit(L"Item file corrupted (legacy format).");
        return false;
    }

    // Decrypt buffer
    ItemDataFileIO::DecryptBuffer(Buffer, Size, MAX_ITEM);

    // Copy items
    BYTE* pSeek = Buffer;
    for (int i = 0; i < MAX_ITEM; i++)
    {
        ITEM_ATTRIBUTE_FILE_LEGACY source;
        memcpy(&source, pSeek, sizeof(source));
        CopyItemAttributeFromSource(ItemAttribute[i], source);
        pSeek += Size;
    }

    delete[] Buffer;
    return true;
}

bool ItemDataLoader::LoadNewFormat(FILE* fp, long fileSize)
{
    const int Size = sizeof(ITEM_ATTRIBUTE_FILE);

    // Read buffer and checksum
    DWORD dwCheckSum;
    BYTE* Buffer = ItemDataFileIO::ReadAndDecryptBuffer(fp, Size, MAX_ITEM, &dwCheckSum);
    if (!Buffer)
    {
        ItemDataFileIO::ShowErrorAndExit(L"Failed to read item file.");
        return false;
    }

    // Verify checksum
    if (!ItemDataFileIO::VerifyChecksum(Buffer, Size * MAX_ITEM, dwCheckSum))
    {
        delete[] Buffer;
        ItemDataFileIO::ShowErrorAndExit(L"Item file corrupted.");
        return false;
    }

    // Decrypt buffer
    ItemDataFileIO::DecryptBuffer(Buffer, Size, MAX_ITEM);

    // Copy items
    BYTE* pSeek = Buffer;
    for (int i = 0; i < MAX_ITEM; i++)
    {
        ITEM_ATTRIBUTE_FILE source;
        memcpy(&source, pSeek, sizeof(source));
        CopyItemAttributeFromSource(ItemAttribute[i], source);
        pSeek += Size;
    }

    delete[] Buffer;
    return true;
}