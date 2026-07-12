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

#include <iterator>

#ifndef _WIN32
#include <string>

#include "GameShop/FileDownloader/CurlFileDownloader.h"

namespace
{
std::wstring BuildFtpUrl(const std::wstring& host, unsigned short port, const std::wstring& remotePath)
{
    // WinINet's FtpOpenFile treated the remote path as relative to the login
    // directory; libcurl's ftp://host/path does the same with a single leading
    // slash. Normalise separators and drop any leading slash so exactly one
    // sits between the host and the path.
    std::wstring path = remotePath;
    for (wchar_t& ch : path)
    {
        if (ch == L'\\')
        {
            ch = L'/';
        }
    }
    while (!path.empty() && path.front() == L'/')
    {
        path.erase(path.begin());
    }

    return L"ftp://" + host + L":" + std::to_wstring(port) + L"/" + path;
}
} // namespace
#endif // _WIN32

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
    // Off Windows the WinINet FileDownloader is replaced by libcurl (issue #462).
    result.BuildSuccessResult();

    wchar_t versionDir[MAX_PATH] = { 0 };
    StringCchPrintf(versionDir, std::size(versionDir), L"%03d.%04d.%03d", Version.Zone, Version.year, Version.yearId);

    const std::wstring remoteBase = strRemotepath + versionDir + L"/";
    const std::wstring localBase = strlocalpath + versionDir + L"/";

    for (std::vector<std::wstring>::iterator it = vScriptFiles.begin(); it != vScriptFiles.end(); ++it)
    {
        const std::wstring localPath = localBase + (*it);
        const std::wstring url = BuildFtpUrl(strServerIP, PortNum, remoteBase + (*it));

        result = CurlFileDownloader::DownloadFile(url, localPath, strUserName, strPWD, bPassiveMode, &this->m_Break);

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

    StringCchPrintf(Buffer, std::size(Buffer), L"%03d.%04d.%03d", Version.Zone, Version.year, Version.yearId);

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