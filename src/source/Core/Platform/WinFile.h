// Portable Win32 file-API shim (issue #462, Phase 3).
//
// A few subsystems do file I/O through the Win32 HANDLE API (CreateFile/ReadFile/
// WriteFile/CloseHandle/SetFilePointer/DeleteFile) and read the clock through
// GetLocalTime. On Windows these come from <windows.h>; elsewhere this maps them
// onto POSIX file descriptors and the C clock. A HANDLE carries the fd value.
#pragma once

#ifdef _WIN32

// File API + SYSTEMTIME come from <windows.h>.

#else  // ---- non-Windows ----------------------------------------------------

#include <fcntl.h>
#include <sys/stat.h>  // stat, mkdir
#include <unistd.h>
#include <cstdlib>   // wcstombs
#include <ctime>
#include <cstdint>
#include "Core/Platform/WinCompat.h"  // HANDLE, DWORD, BOOL, LPCWSTR, WORD, INVALID_HANDLE_VALUE
#include "Core/Platform/WinNls.h"     // WideCharToMultiByte / CP_UTF8 (locale-independent paths)
#include "Core/Platform/PathResolve.h" // MuResolvePath (Windows-spelled asset paths)

// dwDesiredAccess
#ifndef GENERIC_READ
#define GENERIC_READ  0x80000000u
#endif
#ifndef GENERIC_WRITE
#define GENERIC_WRITE 0x40000000u
#endif
// dwShareMode (advisory only on POSIX, ignored here)
#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ   0x00000001
#endif
#ifndef FILE_SHARE_WRITE
#define FILE_SHARE_WRITE  0x00000002
#endif
// dwCreationDisposition
#ifndef CREATE_NEW
#define CREATE_NEW        1
#define CREATE_ALWAYS     2
#define OPEN_EXISTING     3
#define OPEN_ALWAYS       4
#define TRUNCATE_EXISTING 5
#endif
#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#endif
// SetFilePointer move method
#ifndef FILE_BEGIN
#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2
#endif
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER (static_cast<DWORD>(-1))
#endif

namespace mu_detail
{
    inline int    HandleToFd(HANDLE h) { return static_cast<int>(reinterpret_cast<std::intptr_t>(h)); }
    inline HANDLE FdToHandle(int fd)   { return reinterpret_cast<HANDLE>(static_cast<std::intptr_t>(fd)); }
}

inline HANDLE CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD /*share*/, void* /*sec*/,
                          DWORD dwCreationDisposition, DWORD /*flags*/, HANDLE /*tmpl*/)
{
    const bool wantRead  = (dwDesiredAccess & GENERIC_READ) != 0;
    const bool wantWrite = (dwDesiredAccess & GENERIC_WRITE) != 0;
    int oflag = wantRead && wantWrite ? O_RDWR : (wantWrite ? O_WRONLY : O_RDONLY);

    switch (dwCreationDisposition)
    {
        case CREATE_ALWAYS:     oflag |= O_CREAT | O_TRUNC; break;
        case CREATE_NEW:        oflag |= O_CREAT | O_EXCL;  break;
        case OPEN_ALWAYS:       oflag |= O_CREAT;           break;
        case TRUNCATE_EXISTING: oflag |= O_TRUNC;           break;
        case OPEN_EXISTING:
        default:                                            break;
    }

    char path[4096] = { 0 };
    if (!lpFileName || WideCharToMultiByte(CP_UTF8, 0, lpFileName, -1, path, sizeof(path) - 1, nullptr, nullptr) == 0)
        return INVALID_HANDLE_VALUE;

    const int fd = ::open(MuResolvePath(path).c_str(), oflag, 0644);
    return (fd < 0) ? INVALID_HANDLE_VALUE : mu_detail::FdToHandle(fd);
}
#ifndef CreateFile
#define CreateFile CreateFileW
#endif

// A null HANDLE maps to fd 0 (stdin), so guard it: reading/writing/closing it
// would hang on or clobber standard input.
inline bool MuFileHandleValid(HANDLE h) { return h != nullptr && h != INVALID_HANDLE_VALUE; }

inline BOOL ReadFile(HANDLE hFile, void* buffer, DWORD count, LPDWORD bytesRead, void* /*ovl*/)
{
    if (!MuFileHandleValid(hFile)) { if (bytesRead) *bytesRead = 0; return FALSE; }
    const ssize_t r = ::read(mu_detail::HandleToFd(hFile), buffer, count);
    if (r < 0) { if (bytesRead) *bytesRead = 0; return FALSE; }
    if (bytesRead) *bytesRead = static_cast<DWORD>(r);
    return TRUE;
}

inline BOOL WriteFile(HANDLE hFile, const void* buffer, DWORD count, LPDWORD bytesWritten, void* /*ovl*/)
{
    if (!MuFileHandleValid(hFile)) { if (bytesWritten) *bytesWritten = 0; return FALSE; }
    const ssize_t w = ::write(mu_detail::HandleToFd(hFile), buffer, count);
    if (w < 0) { if (bytesWritten) *bytesWritten = 0; return FALSE; }
    if (bytesWritten) *bytesWritten = static_cast<DWORD>(w);
    return TRUE;
}

inline BOOL CloseHandle(HANDLE hObject)
{
    if (!MuFileHandleValid(hObject)) return FALSE;
    return (::close(mu_detail::HandleToFd(hObject)) == 0) ? TRUE : FALSE;
}

inline DWORD SetFilePointer(HANDLE hFile, LONG distance, LONG* distanceHigh, DWORD moveMethod)
{
    if (!MuFileHandleValid(hFile)) return INVALID_SET_FILE_POINTER;
    const int whence = (moveMethod == FILE_END) ? SEEK_END
                     : (moveMethod == FILE_CURRENT) ? SEEK_CUR : SEEK_SET;

    // Combine the optional high dword for offsets past 2GB.
    std::int64_t offset = distance;
    if (distanceHigh)
        offset = (static_cast<std::int64_t>(*distanceHigh) << 32) | static_cast<std::uint32_t>(distance);

    const off_t pos = ::lseek(mu_detail::HandleToFd(hFile), static_cast<off_t>(offset), whence);
    if (pos == static_cast<off_t>(-1)) return INVALID_SET_FILE_POINTER;
    if (distanceHigh) *distanceHigh = static_cast<LONG>(pos >> 32);
    return static_cast<DWORD>(pos & 0xFFFFFFFF);
}

#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE (static_cast<DWORD>(-1))
#endif
inline DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
{
    if (!MuFileHandleValid(hFile)) return INVALID_FILE_SIZE;
    // fstat is a single syscall, thread-safe, and leaves the fd offset
    // untouched (unlike seeking to the end and back).
    struct stat st {};
    if (::fstat(mu_detail::HandleToFd(hFile), &st) != 0) return INVALID_FILE_SIZE;
    if (lpFileSizeHigh) *lpFileSizeHigh = static_cast<DWORD>(static_cast<std::uint64_t>(st.st_size) >> 32);
    return static_cast<DWORD>(static_cast<std::uint64_t>(st.st_size) & 0xFFFFFFFF);
}

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES (static_cast<DWORD>(-1))
#endif
#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif
inline DWORD GetFileAttributesW(LPCWSTR lpFileName)
{
    char path[4096] = { 0 };
    if (!lpFileName || WideCharToMultiByte(CP_UTF8, 0, lpFileName, -1, path, sizeof(path) - 1, nullptr, nullptr) == 0)
        return INVALID_FILE_ATTRIBUTES;
    struct stat st {};
    if (::stat(MuResolvePath(path).c_str(), &st) != 0)
        return INVALID_FILE_ATTRIBUTES;
    return S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
}
#ifndef GetFileAttributes
#define GetFileAttributes GetFileAttributesW
#endif

inline BOOL CreateDirectoryW(LPCWSTR lpPathName, void* /*securityAttributes*/)
{
    char path[4096] = { 0 };
    if (!lpPathName || WideCharToMultiByte(CP_UTF8, 0, lpPathName, -1, path, sizeof(path) - 1, nullptr, nullptr) == 0)
        return FALSE;
    return (::mkdir(MuResolvePath(path).c_str(), 0755) == 0) ? TRUE : FALSE;
}
#ifndef CreateDirectory
#define CreateDirectory CreateDirectoryW
#endif

// Current working directory (Win32 GetCurrentDirectoryW): on success returns the
// length copied (without the terminator); if the buffer is too small or null,
// returns the required size *including* the terminator, like the Win32 contract,
// so callers that query-then-allocate keep working; 0 on failure.
inline DWORD GetCurrentDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer)
{
    char path[4096] = { 0 };
    if (!::getcwd(path, sizeof(path))) return 0;
    const int required = MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0);  // includes the null
    if (required <= 0) return 0;
    if (!lpBuffer || nBufferLength < static_cast<DWORD>(required))
        return static_cast<DWORD>(required);
    const int converted = MultiByteToWideChar(CP_UTF8, 0, path, -1, lpBuffer, static_cast<int>(nBufferLength));
    return (converted > 0) ? static_cast<DWORD>(converted - 1) : 0;
}
#ifndef GetCurrentDirectory
#define GetCurrentDirectory GetCurrentDirectoryW
#endif

inline BOOL DeleteFileW(LPCWSTR lpFileName)
{
    char path[4096] = { 0 };
    if (!lpFileName || WideCharToMultiByte(CP_UTF8, 0, lpFileName, -1, path, sizeof(path) - 1, nullptr, nullptr) == 0)
        return FALSE;
    return (::unlink(MuResolvePath(path).c_str()) == 0) ? TRUE : FALSE;
}
#ifndef DeleteFile
#define DeleteFile DeleteFileW
#endif

typedef struct _SYSTEMTIME {
    WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
} SYSTEMTIME, * LPSYSTEMTIME, * PSYSTEMTIME;

inline void GetLocalTime(LPSYSTEMTIME st)
{
    if (!st) return;
    const time_t t = ::time(nullptr);
    struct tm lt {};
    ::localtime_r(&t, &lt);
    st->wYear         = static_cast<WORD>(lt.tm_year + 1900);
    st->wMonth        = static_cast<WORD>(lt.tm_mon + 1);
    st->wDayOfWeek    = static_cast<WORD>(lt.tm_wday);
    st->wDay          = static_cast<WORD>(lt.tm_mday);
    st->wHour         = static_cast<WORD>(lt.tm_hour);
    st->wMinute       = static_cast<WORD>(lt.tm_min);
    st->wSecond       = static_cast<WORD>(lt.tm_sec);
    st->wMilliseconds = 0;
}

#endif // _WIN32
