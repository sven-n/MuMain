//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: FTPFileDownLoader.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "FTPFileDownLoader.h"
CFTPFileDownLoader::CFTPFileDownLoader() // OK
{
    this->m_Break = 0;
    this->m_pFileDownloader = NULL;
}

CFTPFileDownLoader::~CFTPFileDownLoader() // OK
{
#ifdef _WIN32
    // The WinInet FileDownloader is excluded off Windows (issue #462); the
    // pointer is always null there, and even a delete of a null pointer would
    // still link against the destructor symbol.
    SAFE_DELETE(this->m_pFileDownloader);
#endif
}

WZResult CFTPFileDownLoader::DownLoadFiles(DownloaderType type,
    std::wstring strServerIP,
    unsigned short PortNum,
    std::wstring strUserName,
    std::wstring strPWD,
    std::wstring strRemotepath,
    std::wstring strlocalpath,
    bool bPassiveMode,
    CListVersionInfo Version,
    std::vector<std::wstring>	vScriptFiles) // OK
{
    static WZResult result;

#ifndef _WIN32
    // The WinInet downloader is excluded off Windows (issue #462); report the
    // download as failed so the shop falls back to its no-script state.
    result.SetResult(1, 0, L"File download is not supported on this platform");
    return result;
#else
    result.BuildSuccessResult();

    DownloadServerInfo ServerInfo;
    DownloadFileInfo FileInfo;

    ServerInfo.SetPassiveMode(bPassiveMode);
    ServerInfo.SetOverWrite(1);
    ServerInfo.SetDownloaderType(FTP);
    ServerInfo.SetConnectTimeout(3000);
    ServerInfo.SetServerInfo((TCHAR*)strServerIP.c_str(), PortNum, (TCHAR*)strUserName.c_str(), (TCHAR*)strPWD.c_str());

    wchar_t Buffer[MAX_PATH] = { 0 };

    StringCchPrintf(Buffer, sizeof(Buffer), L"%03d.%04d.%03d", Version.Zone, Version.year, Version.yearId);

    strRemotepath += Buffer;
    strRemotepath += L"/";
    strlocalpath += Buffer;
    strlocalpath += L"\\";

    for (std::vector<std::wstring>::iterator it = vScriptFiles.begin(); it != vScriptFiles.end(); it++)
    {
        std::wstring lPath = strlocalpath + (*it);
        std::wstring rPath = strRemotepath + (*it);

        FileInfo.SetFilePath((TCHAR*)it->c_str(), (TCHAR*)lPath.c_str(), (TCHAR*)rPath.c_str(), NULL);

        this->m_pFileDownloader = new FileDownloader(NULL, &ServerInfo, &FileInfo);

        result = this->m_pFileDownloader->DownloadFile();

        SAFE_DELETE(this->m_pFileDownloader);

        if (this->m_Break != 0)
        {
            result.SetResult(1, 0, L"Time Out Break");
            break;
        }

        if (!result.IsSuccess())
        {
            break;
        }
    }

    return result;
#endif // _WIN32
}

void	CFTPFileDownLoader::Break() // OK
{
    this->m_Break = 1;
#ifdef _WIN32
    if (this->m_pFileDownloader != NULL)
        m_pFileDownloader->Break();
#endif
}

BOOL CFTPFileDownLoader::CreateFolder(std::wstring strFilePath) // OK
{
    if (GetFileAttributes(strFilePath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        return CreateDirectory(strFilePath.c_str(), 0);
    }

    return 1;
}
#endif