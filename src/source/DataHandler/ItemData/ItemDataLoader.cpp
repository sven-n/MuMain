#include "stdafx.h"

#include "ItemDataLoader.h"
#include "ItemDataFileIO.h"
#include "GameData/ItemData/ItemStructs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"
#include "CSChaosCastle.h"
#include <sstream>

#ifdef _EDITOR
#include "UI/Console/MuEditorConsoleUI.h"
#include "Utilities/StringUtils.h"
#endif

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

bool ItemDataLoader::Load(wchar_t* fileName)
{
    FILE* fp = _wfopen(fileName, L"rb");
    if (fp == NULL)
    {
        std::wstringstream ss;
        ss << fileName << L" - File not exist.";
        ItemDataFileIO::ShowErrorAndExit(ss.str().c_str());
        return false;
    }

    // Get file size to determine structure version
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    const int LegacySize = sizeof(ITEM_ATTRIBUTE_FILE_LEGACY);
    const int NewSize = sizeof(ITEM_ATTRIBUTE_FILE);
    const long expectedLegacySize = LegacySize * MAX_ITEM + sizeof(DWORD);
    const long expectedNewSize = NewSize * MAX_ITEM + sizeof(DWORD);

    bool isLegacyFormat = (fileSize == expectedLegacySize);
    bool success = false;

#ifdef _EDITOR
    if (isLegacyFormat)
    {
        g_MuEditorConsoleUI.LogEditor("Detected legacy item format (30-byte names)");
    }
#endif

    if (isLegacyFormat)
    {
        success = LoadLegacyFormat(fp, fileSize);
    }
    else
    {
        success = LoadNewFormat(fp, fileSize);
    }

    fclose(fp);

#ifdef _EDITOR
    if (success)
    {
        // Count non-empty items (items with names)
        int itemCount = 0;
        for (int i = 0; i < MAX_ITEM; i++)
        {
            if (ItemAttribute[i].Name[0] != L'\0')
            {
                itemCount++;
            }
        }

        wchar_t successMsg[256];
        swprintf(successMsg, L"Loaded %d items from %ls", itemCount, fileName);
        g_MuEditorConsoleUI.LogEditor(StringUtils::WideToNarrow(successMsg));
    }
#endif

    return success;
}

template<typename TFileFormat>
bool ItemDataLoader::LoadFormat(FILE* fp, const wchar_t* formatName)
{
    const int Size = sizeof(TFileFormat);

    // Read buffer and checksum
    DWORD dwCheckSum;
    auto buffer = ItemDataFileIO::ReadAndDecryptBuffer(fp, Size, MAX_ITEM, &dwCheckSum);
    if (!buffer)
    {
        std::wstringstream ss;
        ss << L"Failed to read item file (" << formatName << L").";
        ItemDataFileIO::ShowErrorAndExit(ss.str().c_str());
        return false;
    }

    // Verify checksum
    if (!ItemDataFileIO::VerifyChecksum(buffer.get(), Size * MAX_ITEM, dwCheckSum))
    {
        std::wstringstream ss;
        ss << L"Item file corrupted (" << formatName << L").";
        ItemDataFileIO::ShowErrorAndExit(ss.str().c_str());
        return false;
    }

    // Decrypt buffer
    ItemDataFileIO::DecryptBuffer(buffer.get(), Size, MAX_ITEM);

    // Copy items
    BYTE* pSeek = buffer.get();
    for (int i = 0; i < MAX_ITEM; i++)
    {
        TFileFormat source;
        memcpy(&source, pSeek, sizeof(source));
        CopyItemAttributeFromSource(ItemAttribute[i], source);
        pSeek += Size;
    }

    return true;
}

bool ItemDataLoader::LoadLegacyFormat(FILE* fp, long fileSize)
{
    return LoadFormat<ITEM_ATTRIBUTE_FILE_LEGACY>(fp, L"legacy format");
}

bool ItemDataLoader::LoadNewFormat(FILE* fp, long fileSize)
{
    return LoadFormat<ITEM_ATTRIBUTE_FILE>(fp, L"new format");
}