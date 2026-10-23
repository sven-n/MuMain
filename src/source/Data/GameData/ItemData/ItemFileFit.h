#pragma once

#include "ItemStructs.h"
#include "ItemType.h"

#include <limits>
#include <string>
#include <vector>

// A value of an item that its field in the .bmd item files cannot hold: the file keeps damage and magic power in one
// byte, and later items pass 255.
struct ItemFileMisfit
{
    int itemType = 0;
    const char* field = nullptr;
    long long value = 0;
    long long limit = 0;
};

// Every value of items[0..count) that the file's type for its field cannot hold. The types come from the file
// structure, so any field that is narrower in the file is checked. Both file formats have the same field types.
#define FIND_ITEM_FILE_MISFIT(name, type, arraySize, width, i18nName)                                                  \
    {                                                                                                                  \
        using FileType = decltype(ITEM_ATTRIBUTE_FILE::name);                                                          \
        if (static_cast<FileType>(item.name) != item.name)                                                             \
        {                                                                                                              \
            misfits.push_back({itemType, #name, static_cast<long long>(item.name),                                     \
                               static_cast<long long>((std::numeric_limits<FileType>::max)())});                       \
        }                                                                                                              \
    }

inline std::vector<ItemFileMisfit> FindItemFileMisfits(const ITEM_ATTRIBUTE* items, int count)
{
    std::vector<ItemFileMisfit> misfits;
    for (int itemType = 0; itemType < count; ++itemType)
    {
        const ITEM_ATTRIBUTE& item = items[itemType];
        ITEM_FIELDS_SIMPLE(FIND_ITEM_FILE_MISFIT)
    }
    return misfits;
}

#undef FIND_ITEM_FILE_MISFIT

// "Item (0,51) DamageMin is 300; a .bmd item file holds up to 255", one line per misfit, for the editor's console.
inline std::string DescribeItemFileMisfits(const std::vector<ItemFileMisfit>& misfits)
{
    std::string text;
    for (const ItemFileMisfit& misfit : misfits)
    {
        text += "Item (" + std::to_string(Data::Items::GetItemGroup(misfit.itemType)) + "," +
                std::to_string(Data::Items::GetItemNumber(misfit.itemType)) + ") " + misfit.field + " is " +
                std::to_string(misfit.value) + "; a .bmd item file holds up to " + std::to_string(misfit.limit) + "\n";
    }
    return text;
}
