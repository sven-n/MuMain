#include "stdafx.h"

#include "ItemDataLoader.h"
#include "ItemDataFileIO.h"
#include "GameData/ItemData/ItemStructs.h"
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

template<typename TFileFormat>
bool ItemDataLoader::LoadFormat(FILE* fp, const wchar_t* formatName)
{
    const int Size = sizeof(TFileFormat);

    // Read buffer and checksum
    DWORD dwCheckSum;
    auto buffer = ItemDataFileIO::ReadAndDecryptBuffer(fp, Size, MAX_ITEM, &dwCheckSum);
    if (!buffer)
    {
        wchar_t errorMsg[256];
        swprintf(errorMsg, L"Failed to read item file (%ls).", formatName);
        ItemDataFileIO::ShowErrorAndExit(errorMsg);
        return false;
    }

    // Verify checksum
    if (!ItemDataFileIO::VerifyChecksum(buffer.get(), Size * MAX_ITEM, dwCheckSum))
    {
        wchar_t errorMsg[256];
        swprintf(errorMsg, L"Item file corrupted (%ls).", formatName);
        ItemDataFileIO::ShowErrorAndExit(errorMsg);
        return false;
    }

    // Decrypt buffer
    ItemDataFileIO::DecryptBuffer(buffer.get(), Size, MAX_ITEM);

    // Copy items
    BYTE* pSeek = buffer.get();
    for (int i = 0; i < MAX_ITEM; i++)
    {
        TFileFormat source;
        memcpy(&source, pSeek, sizeof(source));
        CopyItemAttributeFromSource(ItemAttribute[i], source);
        pSeek += Size;
    }

    return true;
}

bool ItemDataLoader::LoadLegacyFormat(FILE* fp, long fileSize)
{
    return LoadFormat<ITEM_ATTRIBUTE_FILE_LEGACY>(fp, L"legacy format");
}

bool ItemDataLoader::LoadNewFormat(FILE* fp, long fileSize)
{
    return LoadFormat<ITEM_ATTRIBUTE_FILE>(fp, L"new format");
}