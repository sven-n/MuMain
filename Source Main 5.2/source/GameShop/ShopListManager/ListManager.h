/**************************************************************************************************

작성일: 2009.10.06
작성자: 진혜진

**************************************************************************************************/

#pragma once
#include "include.h"
#include "FTPFileDownLoader.h"

class CListManager
{
public:
    CListManager();
    virtual ~CListManager();

    //				샵 리스트 다운로드 & 관리 를 위한 정보를 설정한다.
    void			SetListManagerInfo(DownloaderType type,
        wchar_t* ServerIP,
        wchar_t* UserID,
        wchar_t* Pwd,
        wchar_t* RemotePath,
        wchar_t* LocalPath,
        CListVersionInfo Version,
        DWORD dwDownloadMaxTime = 0);

    void			SetListManagerInfo(DownloaderType type,
        wchar_t* ServerIP,
        unsigned short PortNum,
        wchar_t* UserID,
        wchar_t* Pwd,
        wchar_t* RemotePath,
        wchar_t* LocalPath,
        FTP_SERVICE_MODE ftpMode,
        CListVersionInfo Version,
        DWORD dwDownloadMaxTime = 0);

    WZResult		LoadScriptList(bool bDonwLoad);

protected:
    bool			IsScriptFileExist();
    std::wstring		GetScriptPath();
    void			DeleteScriptFiles();

    WZResult		FileDownLoad();
    WZResult		FileDownLoadImpl();
    static unsigned int __stdcall RunFileDownLoadThread(LPVOID pParam);

    virtual WZResult LoadScript(bool bDonwLoad) = 0;

    CListManagerInfo			m_ListManagerInfo;
    std::vector<std::wstring>	m_vScriptFiles;
    WZResult					m_Result;
    CFTPFileDownLoader* m_pFTPDownLoader;
};
