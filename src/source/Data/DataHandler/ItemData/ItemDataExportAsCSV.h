#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemStructs.h"

// Item Data Export/Import Operations
class ItemDataExportAsCSV
{
public:
    // Writes the MAX_ITEM records of attributes that have a name.
    static bool ExportToCsv(wchar_t* fileName, const ITEM_ATTRIBUTE* attributes);
    // Future: static bool ImportFromCsv(wchar_t* fileName);
};

#endif // _EDITOR
