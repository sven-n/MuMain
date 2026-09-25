#pragma once

#ifdef _EDITOR

#include <string>

#include "Data/GameData/ItemData/ItemStructs.h"

// Item Data Saving Operations
class ItemDataSaver
{
public:
    // Writes MAX_ITEM records from `items` to an item.bmd file (50-byte
    // names), keeping backups of the old file.
    static bool Save(const wchar_t* fileName, const ITEM_ATTRIBUTE* items, std::string* outChangeLog = nullptr);
};

#endif // _EDITOR
