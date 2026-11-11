#pragma once

#include "Core/Platform/WinCompat.h"
#include <cstddef>
#include <cstdio>
#include <memory>
#include <functional>
#include <optional>
#include <vector>

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
    void ReportError(const wchar_t* message);

    // ReadExact and ReadAll are inline: a loader that only reads must not link DataFileIO.cpp, whose error log
    // (g_ErrorReport) brings the crash reporter, and with it the renderer and the audio, into every binary that
    // links the loader. The unit tests that link the game library without Winmain.cpp would then miss its globals.

    /**
     * Read exactly `size` bytes; reading zero bytes always succeeds
     *
     * @return false when the file ends or a read fails first
     */
    inline bool ReadExact(FILE* fp, void* buffer, std::size_t size)
    {
        return size == 0 || std::fread(buffer, size, 1, fp) == 1;
    }

    /**
     * Read a whole open file, from its start to its end
     *
     * @return std::nullopt when its size cannot be told or a read fails
     */
    inline std::optional<std::vector<BYTE>> ReadAll(FILE* fp)
    {
        if (std::fseek(fp, 0, SEEK_END) != 0)
        {
            return std::nullopt;
        }
        const long size = std::ftell(fp);
        if (size < 0 || std::fseek(fp, 0, SEEK_SET) != 0)
        {
            return std::nullopt;
        }
        std::vector<BYTE> content(static_cast<std::size_t>(size));
        if (!ReadExact(fp, content.data(), content.size()))
        {
            return std::nullopt;
        }
        return content;
    }

    /**
     * Report a game data file that cannot be used, as the loaders report a missing one: the error log, a message box
     * and the game window closing
     *
     * @param fileName The file, as the loader opened it
     * @param issue What is wrong with it, e.g. L"File corrupted."
     */
    void ReportUnusableFile(const wchar_t* fileName, const wchar_t* issue);
}
