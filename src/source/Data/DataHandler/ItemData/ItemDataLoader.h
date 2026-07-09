#pragma once

#include <stdio.h>

// Item Data Loading Operations
class ItemDataLoader
{
public:
    static bool Load(wchar_t* fileName);

private:
    static bool LoadLegacyFormat(FILE* fp, long fileSize);
    static bool LoadNewFormat(FILE* fp, long fileSize);

    // Template for loading item data with different format structures
    template <typename TFileFormat> static bool LoadFormat(FILE* fp, const wchar_t* formatName);
};
