/*******************************************************************************
 *	Path utilities — portable types
 *	Migrated from Win32 (Story 7.6.6)
 *******************************************************************************/

#pragma once

#include <cwchar>
#include <filesystem>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

class Path
{
public:
    static wchar_t* GetCurrentFullPath(wchar_t* szPath);
    static wchar_t* GetCurrentDirectory(wchar_t* szPath);
    static wchar_t* GetCurrentFileName(wchar_t* szPath);

    static wchar_t* SetDirString(wchar_t* szPath);
    static wchar_t* ClearDirString(wchar_t* szPath);

    static wchar_t* GetDirectory(wchar_t* szPath);
    static wchar_t* GetFileName(wchar_t* szPath);

    static wchar_t* ChangeSlashToBackSlash(wchar_t* szPath);
    static wchar_t* ChangeBackSlashToSlash(wchar_t* szPath);

    static bool ReadFileLastLine(wchar_t* szFile, wchar_t* szLastLine);
    static bool WriteNewFile(wchar_t* szFile, wchar_t* szText, int nTextSize);
    static bool CreateDirectorys(wchar_t* szFilePath, bool bIsFile);
};
