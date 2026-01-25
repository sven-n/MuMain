#pragma once

#ifdef _EDITOR

#include <string>

// Item Data Saving Operations
class CommonDataSaver
{
public:
    // static bool Save(wchar_t* fileName, std::string* outChangeLog = nullptr);

    // Create a backup of the file before saving
    static bool CreateBackup(const wchar_t* fileName);

private:
    static constexpr int MaxBackups = 10;

    static void SplitPath(const wchar_t* fileName, std::wstring& outDir, std::wstring& outBaseName);

    static bool BuildBackupName(const wchar_t* fileName, int backupNumber, wchar_t* outBackupName, size_t outBackupNameChars);
    static bool BuildTempBackupName(const wchar_t* fileName, wchar_t* outTempBackupName, size_t outTempBackupNameChars);

    static void EnsureMaxBackupsAfterSuccess(const wchar_t* fileName); // prune only after temp is created
    static int  GetNextBackupNumber(const wchar_t* fileName);

    static bool MatchesBackupPattern(const wchar_t* fileName, const WIN32_FIND_DATAW& fd);
};

#endif // _EDITOR
