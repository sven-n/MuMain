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
};
