#pragma once

#include <stdio.h>

#include <memory>

#include "Data/GameData/ItemData/ItemStructs.h"

// Reads item.bmd files (the legacy item data format).
class ItemDataLoader
{
public:
    // Quiet is for optional files: a missing file is not reported as an error
    // and nothing is written to the editor console. A corrupted file is still
    // reported.
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

        const BYTE* GetRecord(int itemType) const { return records.get() + itemType * recordSize; }
    };

    // Loads MAX_ITEM records from an item.bmd file into destination.
    static bool Load(const wchar_t* fileName, ITEM_ATTRIBUTE* destination, Reporting reporting = Reporting::Normal);

    // Reads, checks and decrypts an item.bmd file without converting it.
    static bool ReadRawFile(const wchar_t* fileName, RawItemFile& file, Reporting reporting = Reporting::Normal);

private:
    static void CopyRecords(const RawItemFile& file, ITEM_ATTRIBUTE* destination);

#ifdef _EDITOR
    static void LogLoadedItems(const wchar_t* fileName, const ITEM_ATTRIBUTE* items, bool isLegacyFormat);
#endif
};
