/**************************************************************************************************

ListManager — download orchestration
Portable types (Story 7.6.6)

**************************************************************************************************/

#pragma once
#include "Include.h"
#include "FTPFileDownLoader.h"

#include <thread>
#include <future>

class CListManager
{
public:
    CListManager();
    virtual ~CListManager();

    void SetListManagerInfo(DownloaderType type, const wchar_t* ServerIP, const wchar_t* UserID, const wchar_t* Pwd,
                            const wchar_t* RemotePath, const wchar_t* LocalPath, CListVersionInfo Version,
                            uint32_t dwDownloadMaxTime = 0);

    void SetListManagerInfo(DownloaderType type, const wchar_t* ServerIP, unsigned short PortNum, const wchar_t* UserID,
                            const wchar_t* Pwd, const wchar_t* RemotePath, const wchar_t* LocalPath,
                            FTP_SERVICE_MODE ftpMode, CListVersionInfo Version, uint32_t dwDownloadMaxTime = 0);

    WZResult LoadScriptList(bool bDonwLoad);

protected:
    bool IsScriptFileExist();
    std::wstring GetScriptPath();
    void DeleteScriptFiles();

    WZResult FileDownLoad();
    WZResult FileDownLoadImpl();

    virtual WZResult LoadScript(bool bDonwLoad) = 0;

    CListManagerInfo m_ListManagerInfo;
    std::vector<std::wstring> m_vScriptFiles;
    WZResult m_Result;
    CFTPFileDownLoader* m_pFTPDownLoader;
};
