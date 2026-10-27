#pragma once

#ifdef _EDITOR

#include <stdio.h>
#include <string>

#include "Data/GameData/ItemData/ItemStructs.h"

// Item Data Legacy Format Saving Operations (S6E3 format)
class ItemDataExportS6E3
{
public:
    // Writes the MAX_ITEM records of attributes. Refuses items with a value its fields cannot hold
    // (FindItemFileMisfits), naming them in `outProblems`.
    static bool SaveLegacy(wchar_t* fileName, const ITEM_ATTRIBUTE* attributes, std::string* outProblems = nullptr);
};

#endif // _EDITOR
