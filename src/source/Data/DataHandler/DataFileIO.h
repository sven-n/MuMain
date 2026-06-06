#pragma once

#include <windows.h>
#include <memory>
#include <functional>

/**
 * Generic file I/O operations for game data
 *
 * Provides unified interface for reading/writing encrypted data files
 * with checksum verification. Works with any data type.
 */
namespace DataFileIO
{
    /**
     * Configuration for file I/O operations
     */
    struct IOConfig
    {
        int itemSize;           // Size of each record
        int itemCount;          // Number of records
        DWORD checksumKey;      // Checksum generation key (e.g., 0xE2F1 for items, 0x5A18 for skills)

        // Optional: Custom encryption/decryption per record (e.g., BuxConvert)
        std::function<void(BYTE*, int)> encryptRecord = nullptr;
        std::function<void(BYTE*, int)> decryptRecord = nullptr;
    };

    /**
     * Read buffer from file with optional decryption
     * Returns unique_ptr for automatic memory management
     *
     * @param fp File pointer (must be opened in binary read mode)
     * @param config I/O configuration
     * @param outChecksum Optional pointer to receive checksum from file
     * @return Unique pointer to buffer, or nullptr on failure
     */
    std::unique_ptr<BYTE[]> ReadBuffer(FILE* fp, const IOConfig& config, DWORD* outChecksum = nullptr);

    /**
     * Verify checksum of buffer
     *
     * @param buffer Data buffer to verify
     * @param config I/O configuration (uses checksumKey)
     * @param expectedChecksum Checksum value from file
     * @return true if checksum matches, false otherwise
     */
    bool VerifyChecksum(const BYTE* buffer, const IOConfig& config, DWORD expectedChecksum);

    /**
     * Decrypt buffer in-place using per-record decryption
     *
     * @param buffer Buffer to decrypt
     * @param config I/O configuration (uses decryptRecord function)
     */
    void DecryptBuffer(BYTE* buffer, const IOConfig& config);

#ifdef _EDITOR
    /**
     * Write buffer to file with encryption and checksum
     *
     * @param fp File pointer (must be opened in binary write mode)
     * @param buffer Data buffer to write
     * @param config I/O configuration
     * @return true on success, false on failure
     */
    bool WriteBuffer(FILE* fp, BYTE* buffer, const IOConfig& config);

    /**
     * Encrypt buffer in-place using per-record encryption
     *
     * @param buffer Buffer to encrypt
     * @param config I/O configuration (uses encryptRecord function)
     */
    void EncryptBuffer(BYTE* buffer, const IOConfig& config);
#endif

    /**
     * Show error message and log to error report
     *
     * @param message Error message to display
     */
    void ShowErrorAndExit(const wchar_t* message);
}
