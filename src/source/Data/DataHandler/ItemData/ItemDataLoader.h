#pragma once

#include <stdio.h>

#include "Data/GameData/ItemData/ItemStructs.h"

// Item Data Loading Operations
class ItemDataLoader
{
public:
    // Loads MAX_ITEM records from an item.bmd file into destination.
    static bool Load(const wchar_t* fileName, ITEM_ATTRIBUTE* destination);

private:
    static bool LoadLegacyFormat(FILE* fp, ITEM_ATTRIBUTE* destination);
    static bool LoadNewFormat(FILE* fp, ITEM_ATTRIBUTE* destination);

    // Template for loading item data with different format structures
    template <typename TFileFormat> static bool LoadFormat(FILE* fp, const wchar_t* formatName, ITEM_ATTRIBUTE* destination);
};
