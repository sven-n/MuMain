#include "stdafx.h"

#ifdef _EDITOR

#include "ItemDataExportS6E3.h"
#include "DataHandler/DataFileIO.h"
#include "GameData/ItemData/ItemStructs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"
#include <memory>

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

bool ItemDataExportS6E3::SaveLegacy(wchar_t* fileName)
{
    const int Size = sizeof(ITEM_ATTRIBUTE_FILE_LEGACY);

    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == NULL)
    {
        return false;
    }

    auto Buffer = std::make_unique<BYTE[]>(Size * MAX_ITEM);
    BYTE* pSeek = Buffer.get();

    // Convert ItemAttribute to ITEM_ATTRIBUTE_FILE_LEGACY format
    for (int i = 0; i < MAX_ITEM; i++)
    {
        ITEM_ATTRIBUTE_FILE_LEGACY dest;
        memset(&dest, 0, Size);

        CopyItemAttributeToDestination(dest, ItemAttribute[i]);

        memcpy(pSeek, &dest, Size);
        pSeek += Size;
    }

    // Configure I/O
    DataFileIO::IOConfig config;
    config.itemSize = Size;
    config.itemCount = MAX_ITEM;
    config.checksumKey = 0xE2F1;
    config.encryptRecord = [](BYTE* data, int size) { BuxConvert(data, size); };

    // Encrypt buffer
    DataFileIO::EncryptBuffer(Buffer.get(), config);

    // Write buffer and checksum
    bool success = DataFileIO::WriteBuffer(fp, Buffer.get(), config);

    fclose(fp);

    return success;
}

#endif // _EDITOR
