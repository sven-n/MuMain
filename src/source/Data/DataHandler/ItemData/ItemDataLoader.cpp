#include "stdafx.h"

#include "ItemDataLoader.h"
#include "Data/DataHandler/DataFileIO.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "Core/Globals/_define.h"
#include "Engine/Object/ZzzInfomation.h"
#include <sstream>

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
} // namespace

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
