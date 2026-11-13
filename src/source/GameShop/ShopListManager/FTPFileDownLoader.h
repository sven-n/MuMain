#pragma once

#include <atomic>

#include "Include.h"

class CFTPFileDownLoader
{
public:
    CFTPFileDownLoader();
    virtual ~CFTPFileDownLoader();

public:
    WZResult DownLoadFiles(DownloaderType type, std::wstring strServerIP, unsigned short PortNum,
                           std::wstring strUserName, std::wstring strPWD, std::wstring strRemotepath,
                           std::wstring strlocalpath, bool bPassiveMode, CListVersionInfo Version,
                           std::vector<std::wstring> vScriptFiles);

    void Break();


private:
    bool CreateFolder(std::wstring strFilePath);
    std::atomic_bool m_Break;
};
