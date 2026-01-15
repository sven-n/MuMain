#include "stdafx.h"

#ifdef _EDITOR

#include "ItemDataSaverLegacy.h"
#include "ItemDataFileIO.h"
#include "GameData/ItemData/ItemStructs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"
#include <memory>

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

bool ItemDataSaverLegacy::SaveLegacy(wchar_t* fileName)
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

    // Encrypt buffer
    ItemDataFileIO::EncryptBuffer(Buffer.get(), Size, MAX_ITEM);

    // Write buffer and checksum
    ItemDataFileIO::WriteAndEncryptBuffer(fp, Buffer.get(), Size * MAX_ITEM);

    fclose(fp);

    return true;
}

#endif // _EDITOR
