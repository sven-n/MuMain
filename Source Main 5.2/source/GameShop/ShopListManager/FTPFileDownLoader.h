#pragma once

#include "include.h"

class CFTPFileDownLoader
{
public:
    CFTPFileDownLoader();
    virtual ~CFTPFileDownLoader();

public:
    WZResult DownLoadFiles(DownloaderType type,
        std::wstring strServerIP,
        unsigned short PortNum,
        std::wstring strUserName,
        std::wstring strPWD,
        std::wstring strRemotepath,
        std::wstring strlocalpath,
        bool bPassiveMode,
        CListVersionInfo Version,
        std::vector<std::wstring>	vScriptFiles);

    void	Break();

    FileDownloader* GetFileDownloader() { return m_pFileDownloader; }

private:
    BOOL CreateFolder(std::wstring strFilePath);
    BOOL m_Break;
    FileDownloader* m_pFileDownloader;
};
