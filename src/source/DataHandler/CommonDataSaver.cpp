#include "stdafx.h"

#ifdef _EDITOR

#include "CommonDataSaver.h"
#include <memory>

#include "UI/Console/MuEditorConsoleUI.h"
#include "Utilities/StringUtils.h"

void CommonDataSaver::SplitPath(const wchar_t* fileName, std::wstring& outDir, std::wstring& outBaseName)
{
    outDir.clear();
    outBaseName.clear();

    if (!fileName)
        return;

    std::wstring full(fileName);
    const size_t pos = full.find_last_of(L"\\/");

    if (pos == std::wstring::npos)
    {
        outBaseName = full;
        return;
    }

    outDir = full.substr(0, pos + 1);
    outBaseName = full.substr(pos + 1);
}

// --- helper: does this file name look like a backup of `fileName`?
bool CommonDataSaver::MatchesBackupPattern(const wchar_t* fileName, const WIN32_FIND_DATAW& fd)
{
    // Expected (current theme):
    // "<base>_N<number>_YyyyyMmmDddhHHmMMsSS.bak"
    // NOTE: fd.cFileName is just the file name (no directory)

    if (!fileName || !fd.cFileName[0])
        return false;

    std::wstring dir, base;
    SplitPath(fileName, dir, base);

    const std::wstring candidate = fd.cFileName;

    const std::wstring prefix = base + L"_N";
    if (candidate.rfind(prefix, 0) != 0)
        return false;

    // Skip temp backups (whatever temp naming you use)
    if (candidate.find(L"_TMP_") != std::wstring::npos || candidate.find(L"_TMP") != std::wstring::npos)
        return false;

    // Must end with ".bak"
    const std::wstring suffix = L".bak";
    if (candidate.size() < suffix.size() || candidate.compare(candidate.size() - suffix.size(), suffix.size(), suffix) != 0)
        return false;

    // Must have "_Y" somewhere after the number (helps avoid matching other random *.bak)
    if (candidate.find(L"_Y") == std::wstring::npos)
        return false;

    return true;
}

// --- helper: build backup name with number + local date/time
bool CommonDataSaver::BuildBackupName(const wchar_t* fileName, int backupNumber, wchar_t* outBackupName, size_t outBackupNameChars)
{
    if (!fileName || !outBackupName || outBackupNameChars == 0)
        return false;

    std::wstring dir, base;
    SplitPath(fileName, dir, base);

    SYSTEMTIME st;
    GetLocalTime(&st);

    // Example:
    // "<dir><base>_N02_Y2026M01D25h00m45s03.bak"
    int written = _snwprintf_s(
        outBackupName,
        outBackupNameChars,
        _TRUNCATE,
        L"%ls%ls_N%02d_Y%04dM%02dD%02dh%02dm%02ds%02d.bak",
        dir.c_str(),
        base.c_str(),
        backupNumber,
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond
    );

    return written > 0;
}

bool CommonDataSaver::BuildTempBackupName(const wchar_t* fileName, wchar_t* outTempBackupName, size_t outTempBackupNameChars)
{
    if (!fileName || !outTempBackupName || outTempBackupNameChars == 0)
        return false;

    std::wstring dir, base;
    SplitPath(fileName, dir, base);

    SYSTEMTIME st;
    GetLocalTime(&st);

    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();

    // Example:
    // "<dir><base>_TMP_Y2026_M01_D25_h00_m45_s03_1234_56.bak"
    int written = _snwprintf_s(
        outTempBackupName,
        outTempBackupNameChars,
        _TRUNCATE,
        L"%ls%ls_TMP_Y%04d_M%02d_D%02d_h%02d_m%02d_s%02d_%lu_%lu.bak",
        dir.c_str(),
        base.c_str(),
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond,
        (unsigned long)pid, (unsigned long)tid
    );

    return written > 0;
}

void CommonDataSaver::EnsureMaxBackupsAfterSuccess(const wchar_t* fileName)
{
    std::wstring dir, base;
    SplitPath(fileName, dir, base);

    const std::wstring pattern = dir + base + L"_N*.bak";

    struct BackupEntry
    {
        std::wstring path;   // full path for DeleteFileW
        FILETIME lastWrite{};
    };

    std::vector<BackupEntry> backups;

    WIN32_FIND_DATAW fd{};
    HANDLE hFind = FindFirstFileW(pattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        if (!MatchesBackupPattern(fileName, fd))
            continue;

        // Build full path for deletion
        backups.push_back(BackupEntry{ dir + fd.cFileName, fd.ftLastWriteTime });

    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);

    // We are called AFTER temp backup creation but BEFORE publishing final backup.
    // Keep room for the new final one: <= MaxBackups - 1.
    while ((int)backups.size() > MaxBackups - 1)
    {
        auto oldestIt = std::min_element(backups.begin(), backups.end(),
            [](const BackupEntry& a, const BackupEntry& b)
            {
                return CompareFileTime(&a.lastWrite, &b.lastWrite) < 0;
            });

        if (oldestIt == backups.end())
            break;

        if (!DeleteFileW(oldestIt->path.c_str()))
        {
            // Best-effort pruning; if deletion fails, stop to avoid infinite loop.
            // (Optional: log warning here)
            break;
        }

        backups.erase(oldestIt);
    }
}

// --- helper: choose next number (1..10...)
int CommonDataSaver::GetNextBackupNumber(const wchar_t* fileName)
{
    std::wstring dir, base;
    SplitPath(fileName, dir, base);

    // Search in directory; fd.cFileName will be base-name only.
    const std::wstring pattern = dir + base + L"_N*.bak";

    WIN32_FIND_DATAW fd{};
    HANDLE hFind = FindFirstFileW(pattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return 1;

    long long maxNum = 0;

    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        if (!MatchesBackupPattern(fileName, fd))
            continue;

        // Parse digits after "<base>_N" until first non-digit
        const std::wstring candidate = fd.cFileName;
        const std::wstring prefix = base + L"_N";

        size_t i = prefix.size();
        long long n = 0;
        bool hasDigits = false;

        while (i < candidate.size())
        {
            const wchar_t ch = candidate[i];
            if (ch < L'0' || ch > L'9')
                break;

            hasDigits = true;
            n = (n * 10) + (ch - L'0');
            ++i;
        }

        if (hasDigits && n > maxNum)
            maxNum = n;

    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);

    // Monotonic / unbounded numbering:
    // next = max + 1 (so after deleting N01, if N10 exists -> next is N11)
    if (maxNum < 0)
        maxNum = 0;

    // Clamp to int range if you want; otherwise keep long long internally.
    if (maxNum >= (long long)INT_MAX)
        return INT_MAX;

    return (int)(maxNum + 1);
}

bool CommonDataSaver::CreateBackup(const wchar_t* fileName)
{
    if (!fileName || !fileName[0])
        return false;

    // If original file doesn't exist, nothing to back up
    DWORD attr = GetFileAttributesW(fileName);
    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
        return true;

    // 1) Create TEMP backup first (no deletion yet)
    wchar_t tempBackupName[1024]{};
    if (!BuildTempBackupName(fileName, tempBackupName, _countof(tempBackupName)))
        return false;

    if (!CopyFileW(fileName, tempBackupName, FALSE))
    {
        g_MuEditorConsoleUI.LogEditor("Warning: Could not create backup file");
        return false;
    }

    // 2) Now that we have a successful new backup, prune old ones
    EnsureMaxBackupsAfterSuccess(fileName);

    // 3) Compute final name (number + date) and "publish" by rename
    const int backupNumber = GetNextBackupNumber(fileName);

    wchar_t finalBackupName[1024]{};
    if (!BuildBackupName(fileName, backupNumber, finalBackupName, _countof(finalBackupName)))
    {
        DeleteFileW(tempBackupName); // don’t leave junk behind
        return false;
    }

    if (!MoveFileExW(tempBackupName, finalBackupName, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        // If rename failed, keep the temp backup rather than deleting older backups again.
        // But we already pruned, so log loudly.
        g_MuEditorConsoleUI.LogEditor("Warning: Backup created but could not be finalized (rename failed)");
        return false;
    }

    g_MuEditorConsoleUI.LogEditor("Backup created " + StringUtils::WideToNarrow(finalBackupName));
    return true;
}

#endif // _EDITOR
