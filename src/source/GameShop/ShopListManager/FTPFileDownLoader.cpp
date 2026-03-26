//************************************************************************
//
// FILE: FTPFileDownLoader.cpp
// Migrated to portable types + std::filesystem (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "FTPFileDownLoader.h"

#include <filesystem>
#include <cwchar>

CFTPFileDownLoader::CFTPFileDownLoader()
{
    this->m_Break = false;
    this->m_pFileDownloader = nullptr;
}

CFTPFileDownLoader::~CFTPFileDownLoader()
{
    SAFE_DELETE(this->m_pFileDownloader);
}

WZResult CFTPFileDownLoader::DownLoadFiles(DownloaderType type, std::wstring strServerIP, unsigned short PortNum,
                                           std::wstring strUserName, std::wstring strPWD, std::wstring strRemotepath,
                                           std::wstring strlocalpath, bool bPassiveMode, CListVersionInfo Version,
                                           std::vector<std::wstring> vScriptFiles)
{
    WZResult result;

    result.BuildSuccessResult();

    DownloadServerInfo ServerInfo;
    DownloadFileInfo FileInfo;

    ServerInfo.SetPassiveMode(bPassiveMode);
    ServerInfo.SetOverWrite(true);
    ServerInfo.SetDownloaderType(FTP);
    ServerInfo.SetConnectTimeout(3000);
    ServerInfo.SetServerInfo((wchar_t*)strServerIP.c_str(), PortNum, (wchar_t*)strUserName.c_str(),
                             (wchar_t*)strPWD.c_str());

    wchar_t Buffer[MAX_PATH] = {0};

    swprintf(Buffer, MAX_PATH, L"%03d.%04d.%03d", Version.Zone, Version.year, Version.yearId);

    strRemotepath += Buffer;
    strRemotepath += L"/";
    strlocalpath += Buffer;
    strlocalpath += L"/";

    for (std::vector<std::wstring>::iterator it = vScriptFiles.begin(); it != vScriptFiles.end(); it++)
    {
        std::wstring lPath = strlocalpath + (*it);
        std::wstring rPath = strRemotepath + (*it);

        FileInfo.SetFilePath((wchar_t*)it->c_str(), (wchar_t*)lPath.c_str(), (wchar_t*)rPath.c_str(), nullptr);

        this->m_pFileDownloader = new FileDownloader(nullptr, &ServerInfo, &FileInfo);

        result = this->m_pFileDownloader->DownloadFile();

        SAFE_DELETE(this->m_pFileDownloader);

        if (this->m_Break)
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

void CFTPFileDownLoader::Break()
{
    this->m_Break = true;
    if (this->m_pFileDownloader != nullptr)
        m_pFileDownloader->Break();
}

bool CFTPFileDownLoader::CreateFolder(std::wstring strFilePath)
{
    std::error_code ec;
    if (!std::filesystem::exists(std::filesystem::path(strFilePath), ec))
    {
        return std::filesystem::create_directory(std::filesystem::path(strFilePath), ec);
    }

    return true;
}
#endif
