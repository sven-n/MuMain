//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: Path.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "Path.h"

#include <fstream>
#include <crtdbg.h>
#include <strsafe.h>

TCHAR* Path::GetCurrentFullPath(TCHAR* szPath)
{
    if (!szPath) return 0;

    GetModuleFileName(0, szPath, MAX_PATH);

    return szPath;
}

TCHAR* Path::GetCurrentDirectory(TCHAR* szPath)
{
    if (!szPath) return 0;

    GetModuleFileName(0, szPath, MAX_PATH);

    wchar_t* chr = wcsrchr(szPath, '\\');

    if (!chr) return 0;

    (*chr) = 0;

    return szPath;
}

TCHAR* Path::GetCurrentFileName(TCHAR* szPath)
{
    if (!szPath) return 0;

    GetModuleFileName(0, szPath, MAX_PATH);

    return Path::GetFileName(szPath);
}

TCHAR* Path::SetDirString(TCHAR* szPath)
{
    if (!szPath) return 0;

    if ((*szPath))
    {
        std::size_t len = 0;
        StringCchLength(szPath, MAX_PATH, &len);

        if (szPath[len - 1] != '\\')
        {
            szPath[len] = '\\';
            szPath[len + 1] = 0;
        }
    }
    else
    {
        szPath[0] = '\\';
        szPath[1] = 0;
    }

    return szPath;
}

TCHAR* Path::ClearDirString(TCHAR* szPath)
{
    if (!szPath || !(*szPath)) return szPath;

    std::size_t len = 0;
    StringCchLength(szPath, MAX_PATH, &len);

    if (len > 0 && (szPath[len - 1] == '\\' || szPath[len - 1] == '/' || szPath[len - 1] == '"'))
    {
        szPath[len - 1] = 0;
    }

    if (len >= 2 && (szPath[len - 2] == '\\' || szPath[len - 2] == '/' || szPath[len - 2] == '"'))
    {
        szPath[len - 2] = 0;
    }

    if (szPath[0] == '"')
    {
        StringCchCopy(szPath, MAX_PATH, szPath + 1);
    }

    return szPath;
}

TCHAR* Path::GetDirectory(TCHAR* szPath)
{
    if (!szPath || !(*szPath)) return szPath;

    wchar_t* chr = wcsrchr(szPath, '\\');

    if (!chr)
        chr = wcsrchr(szPath, L'/');

    if (chr)
        (*chr) = 0;
    else
        StringCchCopy(szPath, MAX_PATH, L".");

    return szPath;
}

TCHAR* Path::GetFileName(TCHAR* szPath)
{
    if (!szPath || !(*szPath)) return szPath;

    wchar_t* chr = wcsrchr(szPath, L'\\');

    if (!chr)
        chr = wcsrchr(szPath, L'/');

    if (chr)
        StringCchCopy(szPath, MAX_PATH, szPath + 1);

    return szPath;
}

TCHAR* Path::ChangeSlashToBackSlash(TCHAR* szPath)
{
    if (!szPath || !(*szPath)) return szPath;

    std::size_t len = 0;
    StringCchLength(szPath, MAX_PATH, &len);

    for (std::size_t n = 0; n < len; n++)
    {
        if (szPath[n] == '\\')
            szPath[n] = '/';
    }

    return szPath;
}

TCHAR* Path::ChangeBackSlashToSlash(TCHAR* szPath)
{
    if (!szPath || !(*szPath)) return szPath;

    std::size_t len = 0;
    StringCchLength(szPath, MAX_PATH, &len);

    for (std::size_t n = 0; n < len; n++)
    {
        if (szPath[n] == '/')
            szPath[n] = '\\';
    }

    return szPath;
}

BOOL			Path::ReadFileLastLine(TCHAR* szFile, TCHAR* szLastLine)
{
    std::ifstream ifs(szFile, std::ifstream::in | std::ifstream::binary);

    char buff[1024] = { 0 };

    if (szFile && szLastLine && *szFile && ifs.is_open())
    {
        std::size_t len = 0;

        while (!ifs.eof())
        {
            ifs.getline(buff, sizeof(buff));

            len = 0;
            StringCchLengthA(buff, sizeof(buff), &len);

            // TODO convert buff to utf8
            //if (len > 1)
            //    StringCchCopy(szLastLine, sizeof(buff), buff);
        }

        ifs.close();

        len = 0;
        StringCchLength(szLastLine, 1024, &len);

        if (len > 1)
        {
            return 1;
        }
    }

    return 0;
}

BOOL			Path::WriteNewFile(TCHAR* szFile, TCHAR* szText, INT nTextSize)
{
    if (!szFile || !szText) return 0;

    std::wfstream ofs(szFile, std::wfstream::out | std::wfstream::trunc | std::wfstream::binary);

    if (ofs.is_open())
    {
        ofs.seekp(0, std::ofstream::end);
        ofs.write(szText, nTextSize);
        ofs.close();

        return 1;
    }

    return 0;
}

BOOL			Path::CreateDirectorys(TCHAR* szFilePath, BOOL bIsFile)

{
    if (!szFilePath || !(*szFilePath)) return 0;

    wchar_t PathName[MAX_PATH] = { 0 };
    wchar_t buff2[MAX_PATH] = { 0 };

    StringCchCopy(PathName, sizeof(PathName), szFilePath);

    if (bIsFile)
        Path::GetDirectory(PathName);

    if (CreateDirectory(PathName, 0) == 0)
    {
        StringCchCopy(buff2, sizeof(buff2), PathName);

        wchar_t* chr1 = wcsrchr(buff2, L'\\');
        wchar_t* chr2 = wcschr(buff2, L'\\');

        if (!chr1 || !chr2 || chr1 == chr2)
            return 0;

        (*chr1) = 0;

        if (!Path::CreateDirectorys(buff2, 0))
            return 0;

        return CreateDirectory(PathName, 0);
    }

    return 0;
}
#endif