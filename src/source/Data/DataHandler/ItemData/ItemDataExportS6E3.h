#pragma once

#ifdef _EDITOR

#include <stdio.h>

#include "Data/GameData/ItemData/ItemStructs.h"

// Item Data Legacy Format Saving Operations (S6E3 format)
class ItemDataExportS6E3
{
public:
    // Writes the MAX_ITEM records of attributes.
    static bool SaveLegacy(wchar_t* fileName, const ITEM_ATTRIBUTE* attributes);
};

#endif // _EDITOR
