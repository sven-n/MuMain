// Portable Win32 directory-enumeration / file-operation shim (issue #462).
//
// The editor's atomic-save/backup code (CommonDataSaver) enumerates backup
// files with FindFirstFileW/FindNextFileW, copies and atomically renames them
// with CopyFileW/MoveFileExW, and sorts them by FILETIME. On Windows these come
// from <windows.h>; elsewhere this maps the used subset onto POSIX (dirent,
// stat, fnmatch, rename). Editor-only, so it is pulled in via the editor sources.
#pragma once

#ifdef _WIN32

// Provided by <windows.h>.

#else  // ---- non-Windows ----------------------------------------------------

#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <string>
#include "Core/Platform/WinCompat.h"   // HANDLE, DWORD, BOOL, MAX_PATH, INVALID_HANDLE_VALUE, FILE_ATTRIBUTE_*
#include "Core/Platform/WinNls.h"      // MultiByteToWideChar / WideCharToMultiByte (CP_UTF8)
#include "Core/Platform/PathResolve.h" // MuResolvePath

// ---- FILETIME ---------------------------------------------------------------
// A 64-bit timestamp split into two DWORDs, like the Win32 struct. The shim
// stores the POSIX mtime (seconds); only the ordering matters to callers
// (sorting backups), not the 1601 epoch.
#ifndef _FILETIME_DEFINED
#define _FILETIME_DEFINED
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, * PFILETIME, * LPFILETIME;
#endif

inline std::uint64_t MuFileTimeValue(const FILETIME& ft)
{
    return (static_cast<std::uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

inline LONG CompareFileTime(const FILETIME* a, const FILETIME* b)
{
    if (!a || !b) return 0;
    const std::uint64_t va = MuFileTimeValue(*a), vb = MuFileTimeValue(*b);
    return (va < vb) ? -1 : (va > vb ? 1 : 0);
}

// ---- WIN32_FIND_DATAW + FindFirstFileW / FindNextFileW / FindClose ----------
typedef struct _WIN32_FIND_DATAW {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    wchar_t  cFileName[MAX_PATH];
    wchar_t  cAlternateFileName[14];
} WIN32_FIND_DATAW, * PWIN32_FIND_DATAW, * LPWIN32_FIND_DATAW;

namespace mu_detail
{
    // Live state for an in-progress directory scan, carried in the HANDLE.
    struct FindState
    {
        DIR*        dir = nullptr;
        std::string dirPathUtf8;   // directory, with trailing '/'
        std::string globUtf8;      // filename wildcard (Win32 *, ? -> fnmatch)
    };

    // Fill `out` for `dirPath/name`; returns false if the entry vanished.
    inline bool FillFindData(const std::string& dirPath, const char* name, WIN32_FIND_DATAW& out)
    {
        std::memset(&out, 0, sizeof(out));
        MultiByteToWideChar(CP_UTF8, 0, name, -1, out.cFileName, MAX_PATH);

        struct stat st {};
        const std::string full = dirPath + name;
        if (::stat(full.c_str(), &st) != 0)
            return false;

        out.dwFileAttributes = S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
        const std::uint64_t mtime = static_cast<std::uint64_t>(st.st_mtime);
        out.ftLastWriteTime.dwLowDateTime  = static_cast<DWORD>(mtime & 0xFFFFFFFF);
        out.ftLastWriteTime.dwHighDateTime = static_cast<DWORD>(mtime >> 32);
        out.nFileSizeLow  = static_cast<DWORD>(st.st_size & 0xFFFFFFFF);
        out.nFileSizeHigh = static_cast<DWORD>(static_cast<std::uint64_t>(st.st_size) >> 32);
        return true;
    }

    // Advance the scan to the next entry matching the glob; fill `out`.
    inline bool FindAdvance(FindState* s, WIN32_FIND_DATAW& out)
    {
        for (;;)
        {
            errno = 0;
            const dirent* e = ::readdir(s->dir);
            if (!e) return false;
            if (std::strcmp(e->d_name, ".") == 0 || std::strcmp(e->d_name, "..") == 0)
                continue;
            if (::fnmatch(s->globUtf8.c_str(), e->d_name, 0) != 0)
                continue;
            if (FillFindData(s->dirPathUtf8, e->d_name, out))
                return true;
        }
    }
}

inline HANDLE FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData)
{
    if (!lpFileName || !lpFindFileData) return INVALID_HANDLE_VALUE;

    char patternUtf8[4096] = { 0 };
    if (WideCharToMultiByte(CP_UTF8, 0, lpFileName, -1, patternUtf8, sizeof(patternUtf8) - 1, nullptr, nullptr) == 0)
        return INVALID_HANDLE_VALUE;

    // Split into directory and filename wildcard (accept either separator).
    std::string pattern(patternUtf8);
    for (char& c : pattern) if (c == '\\') c = '/';
    const std::string::size_type slash = pattern.find_last_of('/');
    std::string dirPath = (slash == std::string::npos) ? std::string("./") : pattern.substr(0, slash + 1);
    const std::string glob = (slash == std::string::npos) ? pattern : pattern.substr(slash + 1);

    auto* s = new mu_detail::FindState();
    // Resolve the directory once and reuse it: FillFindData stat()s
    // dirPathUtf8 + name, so storing the unresolved (wrong-case) path would make
    // every stat fail on a case-sensitive filesystem and skip every entry.
    std::string resolvedDir = MuResolvePath(dirPath.c_str());
    if (!resolvedDir.empty() && resolvedDir.back() != '/')
        resolvedDir += '/';
    s->dir = ::opendir(resolvedDir.c_str());
    if (!s->dir) { delete s; return INVALID_HANDLE_VALUE; }
    s->dirPathUtf8 = resolvedDir;
    s->globUtf8 = glob;

    if (!mu_detail::FindAdvance(s, *lpFindFileData))
    {
        ::closedir(s->dir);
        delete s;
        return INVALID_HANDLE_VALUE;
    }
    return reinterpret_cast<HANDLE>(s);
}

inline BOOL FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData)
{
    if (hFindFile == INVALID_HANDLE_VALUE || !hFindFile || !lpFindFileData) return FALSE;
    auto* s = reinterpret_cast<mu_detail::FindState*>(hFindFile);
    return mu_detail::FindAdvance(s, *lpFindFileData) ? TRUE : FALSE;
}

inline BOOL FindClose(HANDLE hFindFile)
{
    if (hFindFile == INVALID_HANDLE_VALUE || !hFindFile) return FALSE;
    auto* s = reinterpret_cast<mu_detail::FindState*>(hFindFile);
    if (s->dir) ::closedir(s->dir);
    delete s;
    return TRUE;
}
#ifndef FindFirstFile
#define FindFirstFile FindFirstFileW
#define FindNextFile  FindNextFileW
#endif

// ---- CopyFileW / MoveFileExW ------------------------------------------------
inline BOOL CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
{
    char src[4096] = { 0 }, dst[4096] = { 0 };
    if (!lpExistingFileName || !lpNewFileName) return FALSE;
    if (WideCharToMultiByte(CP_UTF8, 0, lpExistingFileName, -1, src, sizeof(src) - 1, nullptr, nullptr) == 0) return FALSE;
    if (WideCharToMultiByte(CP_UTF8, 0, lpNewFileName, -1, dst, sizeof(dst) - 1, nullptr, nullptr) == 0) return FALSE;

    const std::string srcPath = MuResolvePath(src);
    const std::string dstPath = MuResolvePath(dst);
    if (bFailIfExists && ::access(dstPath.c_str(), F_OK) == 0) return FALSE;

    FILE* in = ::fopen(srcPath.c_str(), "rb");
    if (!in) return FALSE;
    FILE* out = ::fopen(dstPath.c_str(), "wb");
    if (!out) { ::fclose(in); return FALSE; }

    char buf[65536];
    size_t n;
    bool ok = true;
    while ((n = ::fread(buf, 1, sizeof(buf), in)) > 0)
        if (::fwrite(buf, 1, n, out) != n) { ok = false; break; }
    if (::ferror(in)) ok = false;
    ::fclose(in);
    if (::fclose(out) != 0) ok = false;
    return ok ? TRUE : FALSE;
}
#ifndef CopyFile
#define CopyFile CopyFileW
#endif

#ifndef MOVEFILE_REPLACE_EXISTING
#define MOVEFILE_REPLACE_EXISTING 0x00000001
#define MOVEFILE_COPY_ALLOWED     0x00000002
#define MOVEFILE_WRITE_THROUGH    0x00000008
#endif
inline BOOL MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD /*dwFlags*/)
{
    char src[4096] = { 0 }, dst[4096] = { 0 };
    if (!lpExistingFileName || !lpNewFileName) return FALSE;
    if (WideCharToMultiByte(CP_UTF8, 0, lpExistingFileName, -1, src, sizeof(src) - 1, nullptr, nullptr) == 0) return FALSE;
    if (WideCharToMultiByte(CP_UTF8, 0, lpNewFileName, -1, dst, sizeof(dst) - 1, nullptr, nullptr) == 0) return FALSE;
    // rename(2) replaces the destination atomically, which covers the
    // REPLACE_EXISTING | WRITE_THROUGH the caller uses to publish a backup.
    return (::rename(MuResolvePath(src).c_str(), MuResolvePath(dst).c_str()) == 0) ? TRUE : FALSE;
}
#ifndef MoveFileEx
#define MoveFileEx MoveFileExW
#endif

#endif // _WIN32
