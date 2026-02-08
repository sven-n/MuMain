#include "stdafx.h"
#include "DataFileIO.h"
#include "ZzzInfomation.h"
#include <windows.h>

extern HWND g_hWnd;

namespace DataFileIO
{
    std::unique_ptr<BYTE[]> ReadBuffer(FILE* fp, const IOConfig& config, DWORD* outChecksum)
    {
        int bufferSize = config.itemSize * config.itemCount;
        auto buffer = std::make_unique<BYTE[]>(bufferSize);

        // Read buffer
        size_t bytesRead = fread(buffer.get(), bufferSize, 1, fp);
        if (bytesRead != 1)
        {
            ShowErrorAndExit(L"Failed to read data from file");
            return nullptr;
        }

        // Read checksum if requested
        if (outChecksum)
        {
            bytesRead = fread(outChecksum, sizeof(DWORD), 1, fp);
            if (bytesRead != 1)
            {
                ShowErrorAndExit(L"Failed to read checksum from file");
                return nullptr;
            }
        }

        return buffer;
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

    void ShowErrorAndExit(const wchar_t* message)
    {
        g_ErrorReport.Write(message);
        MessageBox(g_hWnd, message, L"Data File Error", MB_OK | MB_ICONERROR);
        // Note: Application continues running - error handling left to caller
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
}
