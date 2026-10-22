#pragma once

#ifdef _EDITOR

#include <stdio.h>
#include <string>

// Item Data Legacy Format Saving Operations (S6E3 format)
class ItemDataExportS6E3
{
public:
    // Refuses items with a value its fields cannot hold (FindItemFileMisfits), naming them in `outProblems`.
    static bool SaveLegacy(wchar_t* fileName, std::string* outProblems = nullptr);
};

#endif // _EDITOR
