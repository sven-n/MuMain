#include "stdafx.h"
#include "Core/Platform/Quit.h"
#include "DataFileIO.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Core/Platform/WinCompat.h"

namespace DataFileIO
{
template <typename Source>
std::unique_ptr<BYTE[]> ReadBufferFrom(Source* source, const IOConfig& config, DWORD* outChecksum)
{
    int bufferSize = config.itemSize * config.itemCount;
    auto buffer = std::make_unique<BYTE[]>(bufferSize);

    if (!ReadExact(source, buffer.get(), static_cast<std::size_t>(bufferSize)))
    {
        ReportError(L"Failed to read data from file");
        return nullptr;
    }

    if (outChecksum && !ReadExact(source, outChecksum, sizeof(DWORD)))
    {
        ReportError(L"Failed to read checksum from file");
        return nullptr;
    }

    return buffer;
}

std::unique_ptr<BYTE[]> ReadBuffer(FILE* fp, const IOConfig& config, DWORD* outChecksum)
{
    return ReadBufferFrom(fp, config, outChecksum);
}

std::unique_ptr<BYTE[]> ReadBuffer(SDL_IOStream* stream, const IOConfig& config, DWORD* outChecksum)
{
    return ReadBufferFrom(stream, config, outChecksum);
}

bool VerifyChecksum(const BYTE* buffer, const IOConfig& config, DWORD expectedChecksum)
{
    int bufferSize = config.itemSize * config.itemCount;
    DWORD calculatedChecksum = GenerateCheckSum2(buffer, bufferSize, config.checksumKey);
    return (calculatedChecksum == expectedChecksum);
}

void DecryptBuffer(BYTE* buffer, const IOConfig& config)
{
    if (!config.decryptRecord)
    {
        return; // No decryption needed
    }

    BYTE* pSeek = buffer;
    for (int i = 0; i < config.itemCount; i++)
    {
        config.decryptRecord(pSeek, config.itemSize);
        pSeek += config.itemSize;
    }
}

void ReportError(const wchar_t* message)
{
    g_ErrorReport.Write(L"[DataFileIO] %ls\r\n", message);
}

void ReportUnusableFile(const wchar_t* fileName, const wchar_t* issue)
{
    wchar_t text[256]{};
    mu_swprintf(text, L"%ls - %ls", fileName, issue);
    g_ErrorReport.Write(L"%ls\r\n", text);
    Core::Platform::ShowFatalError(L"Error", text);
}

#ifdef _EDITOR
bool WriteBuffer(FILE* fp, BYTE* buffer, const IOConfig& config)
{
    int bufferSize = config.itemSize * config.itemCount;

    // Write buffer
    size_t bytesWritten = fwrite(buffer, bufferSize, 1, fp);
    if (bytesWritten != 1)
    {
        return false;
    }

    // Write checksum
    DWORD dwCheckSum = GenerateCheckSum2(buffer, bufferSize, config.checksumKey);
    bytesWritten = fwrite(&dwCheckSum, sizeof(DWORD), 1, fp);
    if (bytesWritten != 1)
    {
        return false;
    }

    return true;
}

void EncryptBuffer(BYTE* buffer, const IOConfig& config)
{
    if (!config.encryptRecord)
    {
        return; // No encryption needed
    }

    BYTE* pSeek = buffer;
    for (int i = 0; i < config.itemCount; i++)
    {
        config.encryptRecord(pSeek, config.itemSize);
        pSeek += config.itemSize;
    }
}
#endif
} // namespace DataFileIO
