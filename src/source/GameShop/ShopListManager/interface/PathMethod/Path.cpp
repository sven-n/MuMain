//************************************************************************
//
// FILE: Path.cpp
// Migrated to std::filesystem + portable types (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "Path.h"

#include <fstream>
#include <cstring>
#include <cwchar>
#include <filesystem>

wchar_t* Path::GetCurrentFullPath(wchar_t* szPath)
{
    if (!szPath)
        return nullptr;

    std::error_code ec;
    auto exePath = std::filesystem::current_path(ec);
    if (!ec)
    {
        std::wstring ws = exePath.wstring();
        wcsncpy(szPath, ws.c_str(), MAX_PATH - 1);
        szPath[MAX_PATH - 1] = L'\0';
    }
    else
    {
        szPath[0] = L'\0';
    }

    return szPath;
}

wchar_t* Path::GetCurrentDirectory(wchar_t* szPath)
{
    if (!szPath)
        return nullptr;

    std::error_code ec;
    auto curDir = std::filesystem::current_path(ec);
    if (!ec)
    {
        std::wstring ws = curDir.wstring();
        wcsncpy(szPath, ws.c_str(), MAX_PATH - 1);
        szPath[MAX_PATH - 1] = L'\0';
    }
    else
    {
        szPath[0] = L'\0';
    }

    return szPath;
}

wchar_t* Path::GetCurrentFileName(wchar_t* szPath)
{
    if (!szPath)
        return nullptr;

    Path::GetCurrentFullPath(szPath);
    return Path::GetFileName(szPath);
}

wchar_t* Path::SetDirString(wchar_t* szPath)
{
    if (!szPath)
        return nullptr;

    if ((*szPath))
    {
        std::size_t len = wcslen(szPath);

        if (szPath[len - 1] != L'/' && szPath[len - 1] != L'\\')
        {
            szPath[len] = L'/';
            szPath[len + 1] = L'\0';
        }
    }
    else
    {
        szPath[0] = L'/';
        szPath[1] = L'\0';
    }

    return szPath;
}

wchar_t* Path::ClearDirString(wchar_t* szPath)
{
    if (!szPath || !(*szPath))
        return szPath;

    std::size_t len = wcslen(szPath);

    if (len > 0 && (szPath[len - 1] == L'\\' || szPath[len - 1] == L'/' || szPath[len - 1] == L'"'))
    {
        szPath[len - 1] = L'\0';
    }

    if (len >= 2 && (szPath[len - 2] == L'\\' || szPath[len - 2] == L'/' || szPath[len - 2] == L'"'))
    {
        szPath[len - 2] = L'\0';
    }

    if (szPath[0] == L'"')
    {
        std::size_t newLen = wcslen(szPath + 1);
        wmemmove(szPath, szPath + 1, newLen + 1);
    }

    return szPath;
}

wchar_t* Path::GetDirectory(wchar_t* szPath)
{
    if (!szPath || !(*szPath))
        return szPath;

    wchar_t* chr = wcsrchr(szPath, L'\\');

    if (!chr)
        chr = wcsrchr(szPath, L'/');

    if (chr)
        (*chr) = L'\0';
    else
    {
        szPath[0] = L'.';
        szPath[1] = L'\0';
    }

    return szPath;
}

wchar_t* Path::GetFileName(wchar_t* szPath)
{
    if (!szPath || !(*szPath))
        return szPath;

    wchar_t* chr = wcsrchr(szPath, L'\\');

    if (!chr)
        chr = wcsrchr(szPath, L'/');

    if (chr)
    {
        std::size_t len = wcslen(chr + 1);
        wmemmove(szPath, chr + 1, len + 1);
    }

    return szPath;
}

wchar_t* Path::ChangeSlashToBackSlash(wchar_t* szPath)
{
    if (!szPath || !(*szPath))
        return szPath;

    std::size_t len = wcslen(szPath);

    for (std::size_t n = 0; n < len; n++)
    {
        if (szPath[n] == L'\\')
            szPath[n] = L'/';
    }

    return szPath;
}

wchar_t* Path::ChangeBackSlashToSlash(wchar_t* szPath)
{
    if (!szPath || !(*szPath))
        return szPath;

    std::size_t len = wcslen(szPath);

    for (std::size_t n = 0; n < len; n++)
    {
        if (szPath[n] == L'/')
            szPath[n] = L'\\';
    }

    return szPath;
}

bool Path::ReadFileLastLine(wchar_t* szFile, wchar_t* szLastLine)
{
    if (!szFile || !szLastLine || !*szFile)
    {
        return false;
    }

    char narrowPath[MAX_PATH * 4] = {0};
    wcstombs(narrowPath, szFile, sizeof(narrowPath) - 1);

    std::ifstream ifs(narrowPath, std::ifstream::in | std::ifstream::binary);

    char buff[1024] = {0};

    if (ifs.is_open())
    {
        while (!ifs.eof())
        {
            ifs.getline(buff, sizeof(buff));
        }

        ifs.close();

        // Convert the last line read from char to wchar_t using portable mbstowcs
        size_t wideLen = mbstowcs(nullptr, buff, 0);
        if (wideLen > 0 && wideLen != static_cast<size_t>(-1))
        {
            mbstowcs(szLastLine, buff, wideLen + 1);
            std::size_t len = wcslen(szLastLine);

            if (len > 0)
            {
                return true;
            }
        }
    }

    return false;
}

bool Path::WriteNewFile(wchar_t* szFile, wchar_t* szText, int nTextSize)
{
    if (!szFile || !szText)
        return false;

    char narrowPath[MAX_PATH * 4] = {0};
    wcstombs(narrowPath, szFile, sizeof(narrowPath) - 1);

    std::wfstream ofs(narrowPath, std::wfstream::out | std::wfstream::trunc | std::wfstream::binary);

    if (ofs.is_open())
    {
        ofs.seekp(0, std::ofstream::end);
        ofs.write(szText, nTextSize);
        ofs.close();

        return true;
    }

    return false;
}

bool Path::CreateDirectorys(wchar_t* szFilePath, bool bIsFile)
{
    if (!szFilePath || !(*szFilePath))
        return false;

    std::filesystem::path fsPath(szFilePath);

    if (bIsFile)
        fsPath = fsPath.parent_path();

    if (fsPath.empty())
        return false;

    std::error_code ec;
    std::filesystem::create_directories(fsPath, ec);

    return !ec;
}
#endif
