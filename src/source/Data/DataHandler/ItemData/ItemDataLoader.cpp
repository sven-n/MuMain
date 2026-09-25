#include "stdafx.h"

#include "ItemDataLoader.h"
#include "Data/DataHandler/DataFileIO.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "Core/Globals/_struct.h"
#include "Core/Globals/_define.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Data/Translation/MultiLanguage.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include <sstream>

#ifdef _EDITOR
#include "UI/Console/MuEditorConsoleUI.h"
#include "Core/Utilities/StringUtils.h"
#endif

namespace
{
constexpr DWORD ItemFileChecksumKey = 0xE2F1;

void ReportFileError(const wchar_t* fileName, const wchar_t* problem)
{
    std::wstringstream ss;
    ss << fileName << L" - " << problem;
    DataFileIO::ReportError(ss.str().c_str());
}

long GetFileSize(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    const long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return fileSize;
}

bool ReadRecords(FILE* fp, const wchar_t* fileName, ItemDataLoader::RawItemFile& file)
{
    DataFileIO::IOConfig config;
    config.itemSize = file.recordSize;
    config.itemCount = MAX_ITEM;
    config.checksumKey = ItemFileChecksumKey;
    config.decryptRecord = [](BYTE* data, int size) { BuxConvert(data, size); };

    DWORD checksum;
    auto buffer = DataFileIO::ReadBuffer(fp, config, &checksum);
    if (!buffer)
    {
        ReportFileError(fileName, L"Failed to read item file.");
        return false;
    }

    if (!DataFileIO::VerifyChecksum(buffer.get(), config, checksum))
    {
        ReportFileError(fileName, L"Item file corrupted.");
        return false;
    }

    DataFileIO::DecryptBuffer(buffer.get(), config);
    file.records = std::move(buffer);
    return true;
}

template <typename TFileFormat> void CopyRecordsAs(const ItemDataLoader::RawItemFile& file, ITEM_ATTRIBUTE* destination)
{
    for (int i = 0; i < MAX_ITEM; i++)
    {
        TFileFormat source;
        memcpy(&source, file.GetRecord(i), sizeof(source));
        CopyItemAttributeFromSource(destination[i], source);
    }
}
} // namespace

bool ItemDataLoader::Load(const wchar_t* fileName, ITEM_ATTRIBUTE* destination, Reporting reporting)
{
    RawItemFile file;
    if (!ReadRawFile(fileName, file, reporting))
    {
        return false;
    }

    CopyRecords(file, destination);

#ifdef _EDITOR
    if (reporting == Reporting::Normal)
    {
        LogLoadedItems(fileName, destination, file.isLegacyFormat);
    }
#endif

    return true;
}

bool ItemDataLoader::ReadRawFile(const wchar_t* fileName, RawItemFile& file, Reporting reporting)
{
    FILE* fp = _wfopen(fileName, L"rb");
    if (fp == NULL)
    {
        if (reporting == Reporting::Normal)
        {
            ReportFileError(fileName, L"File not exist.");
        }
        return false;
    }

    const long expectedLegacySize = static_cast<long>(sizeof(ITEM_ATTRIBUTE_FILE_LEGACY)) * MAX_ITEM + sizeof(DWORD);
    file.isLegacyFormat = GetFileSize(fp) == expectedLegacySize;
    file.recordSize = file.isLegacyFormat ? sizeof(ITEM_ATTRIBUTE_FILE_LEGACY) : sizeof(ITEM_ATTRIBUTE_FILE);

    const bool success = ReadRecords(fp, fileName, file);
    fclose(fp);
    return success;
}

void ItemDataLoader::CopyRecords(const RawItemFile& file, ITEM_ATTRIBUTE* destination)
{
    if (file.isLegacyFormat)
    {
        CopyRecordsAs<ITEM_ATTRIBUTE_FILE_LEGACY>(file, destination);
    }
    else
    {
        CopyRecordsAs<ITEM_ATTRIBUTE_FILE>(file, destination);
    }
}

#ifdef _EDITOR
void ItemDataLoader::LogLoadedItems(const wchar_t* fileName, const ITEM_ATTRIBUTE* items, bool isLegacyFormat)
{
    if (isLegacyFormat)
    {
        g_MuEditorConsoleUI.LogEditor("Detected legacy item format (30-byte names)");
    }

    // Count non-empty items (items with names)
    int itemCount = 0;
    for (int i = 0; i < MAX_ITEM; i++)
    {
        if (items[i].Name[0] != L'\0')
        {
            itemCount++;
        }
    }

    wchar_t successMsg[256];
    mu_swprintf(successMsg, L"Loaded %d items from %ls", itemCount, fileName);
    g_MuEditorConsoleUI.LogEditor(StringUtils::WideToNarrow(successMsg));
}
#endif
