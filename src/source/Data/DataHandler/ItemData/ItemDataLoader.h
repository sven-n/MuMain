#pragma once

#include <stdio.h>

#include "Data/GameData/ItemData/ItemStructs.h"

// Item Data Loading Operations
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

    // Loads MAX_ITEM records from an item.bmd file into destination.
    static bool Load(const wchar_t* fileName, ITEM_ATTRIBUTE* destination, Reporting reporting = Reporting::Normal);

private:
#ifdef _EDITOR
    static void LogLoadedItems(const wchar_t* fileName, const ITEM_ATTRIBUTE* items, bool isLegacyFormat);
#endif

    static bool LoadLegacyFormat(FILE* fp, ITEM_ATTRIBUTE* destination);
    static bool LoadNewFormat(FILE* fp, ITEM_ATTRIBUTE* destination);

    // Template for loading item data with different format structures
    template <typename TFileFormat> static bool LoadFormat(FILE* fp, const wchar_t* formatName, ITEM_ATTRIBUTE* destination);
};
