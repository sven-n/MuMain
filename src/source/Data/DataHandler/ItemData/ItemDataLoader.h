#pragma once

#include <stdio.h>

#include <memory>

#include "Data/GameData/ItemData/ItemStructs.h"

// Reads legacy item.bmd files. The game itself reads its items from
// Data/Items (JSON); this is only used by the item editor's bmd import.
class ItemDataLoader
{
public:
    // Quiet is for optional files: a missing file is not reported as an
    // error. A corrupted file is still reported.
    enum class Reporting
    {
        Normal,
        Quiet,
    };

    // The decrypted records of an item.bmd file: MAX_ITEM records of
    // ITEM_ATTRIBUTE_FILE_LEGACY (30-byte names) or ITEM_ATTRIBUTE_FILE.
    struct RawItemFile
    {
        std::unique_ptr<BYTE[]> records;
        int recordSize = 0;
        bool isLegacyFormat = false;

        const BYTE* GetRecord(int itemType) const
        {
            return records.get() + itemType * recordSize;
        }
    };

    // Reads, checks and decrypts an item.bmd file without converting it.
    static bool ReadRawFile(const wchar_t* fileName, RawItemFile& file, Reporting reporting = Reporting::Normal);
};
