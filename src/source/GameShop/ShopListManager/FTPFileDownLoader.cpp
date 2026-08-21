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

CFTPFileDownLoader::CFTPFileDownLoader() // OK
{
    this->m_Break = 0;
    this->m_pFileDownloader = NULL;
}

CFTPFileDownLoader::~CFTPFileDownLoader() // OK
{
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
}

void	CFTPFileDownLoader::Break() // OK
{
    this->m_Break = 1;
}

bool CFTPFileDownLoader::CreateFolder(std::wstring strFilePath) // OK
{
    if (GetFileAttributes(strFilePath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        return CreateDirectory(strFilePath.c_str(), 0);
    }

    return 1;
}
#endif
