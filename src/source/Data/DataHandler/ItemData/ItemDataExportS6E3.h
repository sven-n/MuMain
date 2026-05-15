#pragma once

#ifdef _EDITOR

#include <stdio.h>

// Item Data Legacy Format Saving Operations (S6E3 format)
class ItemDataExportS6E3
{
public:
    static bool SaveLegacy(wchar_t* fileName);
};

#endif // _EDITOR
