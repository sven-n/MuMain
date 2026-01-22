#pragma once

#include <windows.h>
#include <memory>
#include <vector>

// Common file I/O operations for item data
namespace ItemDataFileIO
{
    // Read buffer from file with decryption
    // Returns unique_ptr for automatic memory management
    std::unique_ptr<BYTE[]> ReadAndDecryptBuffer(FILE* fp, int itemSize, int itemCount, DWORD* outChecksum = nullptr);

    // Verify checksum of buffer
    bool VerifyChecksum(BYTE* buffer, int bufferSize, DWORD expectedChecksum);

    // Decrypt buffer in-place (apply BuxConvert to each item)
    void DecryptBuffer(BYTE* buffer, int itemSize, int itemCount);

    // Show error message and exit
    void ShowErrorAndExit(const wchar_t* message);

#ifdef _EDITOR
    // Write and encrypt buffer to file with checksum
    bool WriteAndEncryptBuffer(FILE* fp, BYTE* buffer, int bufferSize);

    // Encrypt buffer in-place (apply BuxConvert to each item)
    void EncryptBuffer(BYTE* buffer, int itemSize, int itemCount);
#endif
}
