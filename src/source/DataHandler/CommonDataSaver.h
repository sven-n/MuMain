#pragma once

#ifdef _EDITOR

#include <string>
#include <memory>
#include <functional>

// Generic Data Saving Operations
class CommonDataSaver
{
public:
    // Create a backup of the file before saving
    static bool CreateBackup(const wchar_t* fileName);

    // Generic save configuration
    template<typename TRuntime, typename TFile>
    struct SaveConfig
    {
        // File I/O
        const wchar_t* fileName;
        size_t itemCount;

        // Data pointers
        const TRuntime* runtimeData;  // Current in-memory data

        // Conversion functions
        std::function<void(TFile&, const TRuntime&)> convertToFile;  // Runtime -> File format
        std::function<void(TRuntime&, const TFile&)> convertFromFile; // File -> Runtime format

        // Legacy format support (optional)
        size_t legacyFileStructSize = 0;  // Set to sizeof(legacy struct) if legacy support needed
        std::function<void(TRuntime&, BYTE*, size_t)> convertFromFileLegacy; // Legacy file -> Runtime

        // Encryption/Decryption (optional, can be nullptr for no encryption)
        std::function<void(BYTE*, std::size_t)> encryptBuffer;   // Called once on entire buffer
        std::function<void(BYTE*, std::size_t)> decryptBuffer;   // Called once on entire buffer
        std::function<void(BYTE*, std::size_t)> encryptRecord;   // Called per-record (like BuxConvert)
        std::function<void(BYTE*, std::size_t)> decryptRecord;   // Called per-record (like BuxConvert)

        // Checksum (optional)
        std::function<DWORD(BYTE*, std::size_t)> generateChecksum;

        // Change tracking (optional)
        std::function<void(const TRuntime&, const TRuntime&, std::stringstream&, bool&)> compareItems;
        std::function<std::string(int, const TRuntime&)> getItemName;

        // Change log output
        std::string* outChangeLog = nullptr;
    };

    // Generic save method
    template<typename TRuntime, typename TFile>
    static bool SaveData(const SaveConfig<TRuntime, TFile>& config);

private:
    static constexpr int MaxBackups = 10;

    static void SplitPath(const wchar_t* fileName, std::wstring& outDir, std::wstring& outBaseName);

    static bool BuildBackupName(const wchar_t* fileName, int backupNumber, wchar_t* outBackupName, size_t outBackupNameChars);
    static bool BuildTempBackupName(const wchar_t* fileName, wchar_t* outTempBackupName, size_t outTempBackupNameChars);

    static void EnsureMaxBackupsAfterSuccess(const wchar_t* fileName); // prune only after temp is created
    static int  GetNextBackupNumber(const wchar_t* fileName);

    static bool MatchesBackupPattern(const wchar_t* fileName, const WIN32_FIND_DATAW& fd);
};

// Template implementation (must be in header)
#include "CommonDataSaver.inl"

#endif // _EDITOR
