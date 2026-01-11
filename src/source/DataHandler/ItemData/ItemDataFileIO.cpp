#include "stdafx.h"
#include "ItemDataFileIO.h"
#include "ZzzInfomation.h"
#include <Windows.h>

extern HWND g_hWnd;

namespace ItemDataFileIO
{
    BYTE* ReadAndDecryptBuffer(FILE* fp, int itemSize, int itemCount, DWORD* outChecksum)
    {
        int bufferSize = itemSize * itemCount;
        BYTE* buffer = new BYTE[bufferSize];

        // Read buffer
        size_t bytesRead = fread(buffer, bufferSize, 1, fp);
        if (bytesRead != 1)
        {
            delete[] buffer;
            ShowErrorAndExit(L"Failed to read item data from file");
            return nullptr;
        }

        // Read checksum if requested
        if (outChecksum)
        {
            bytesRead = fread(outChecksum, sizeof(DWORD), 1, fp);
            if (bytesRead != 1)
            {
                delete[] buffer;
                ShowErrorAndExit(L"Failed to read checksum from file");
                return nullptr;
            }
        }

        return buffer;
    }

    bool VerifyChecksum(BYTE* buffer, int bufferSize, DWORD expectedChecksum)
    {
        DWORD calculatedChecksum = GenerateCheckSum2(buffer, bufferSize, 0xE2F1);
        return (calculatedChecksum == expectedChecksum);
    }

    void DecryptBuffer(BYTE* buffer, int itemSize, int itemCount)
    {
        BYTE* pSeek = buffer;
        for (int i = 0; i < itemCount; i++)
        {
            BuxConvert(pSeek, itemSize);
            pSeek += itemSize;
        }
    }

    void ShowErrorAndExit(const wchar_t* message)
    {
        g_ErrorReport.Write(message);
        MessageBox(g_hWnd, message, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
    }

#ifdef _EDITOR
    bool WriteAndEncryptBuffer(FILE* fp, BYTE* buffer, int bufferSize)
    {
        // Write buffer
        fwrite(buffer, bufferSize, 1, fp);

        // Write checksum
        DWORD dwCheckSum = GenerateCheckSum2(buffer, bufferSize, 0xE2F1);
        fwrite(&dwCheckSum, sizeof(DWORD), 1, fp);

        return true;
    }

    void EncryptBuffer(BYTE* buffer, int itemSize, int itemCount)
    {
        BYTE* pSeek = buffer;
        for (int i = 0; i < itemCount; i++)
        {
            BuxConvert(pSeek, itemSize);
            pSeek += itemSize;
        }
    }
#endif
}
