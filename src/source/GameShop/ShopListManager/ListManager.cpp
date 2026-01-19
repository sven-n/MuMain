//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: ListManager.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ListManager.h"
#include <process.h>

CListManager::CListManager() // OK
{
    this->m_pFTPDownLoader = NULL;
    this->m_ListManagerInfo.m_strServerIP.clear();
    this->m_ListManagerInfo.m_strUserID.clear();
    this->m_ListManagerInfo.m_strPWD.clear();
    this->m_ListManagerInfo.m_strRemotePath.clear();
    this->m_ListManagerInfo.m_strLocalPath.clear();
    this->m_vScriptFiles.clear();
}

CListManager::~CListManager() // OK
{
    SAFE_DELETE(m_pFTPDownLoader);
}

void			CListManager::SetListManagerInfo(DownloaderType type,
    wchar_t* ServerIP,
    wchar_t* UserID,
    wchar_t* Pwd,
    wchar_t* RemotePath,
    wchar_t* LocalPath,
    CListVersionInfo Version,
    DWORD dwDownloadMaxTime)
{
    unsigned short port = 80;

    if (type == FTP)
        port = 21;

    this->SetListManagerInfo(type, ServerIP, port, UserID, Pwd, RemotePath, LocalPath, FTP_MODE_ACTIVE, Version, dwDownloadMaxTime);
}

void			CListManager::SetListManagerInfo(DownloaderType type,
    wchar_t* ServerIP,
    unsigned short PortNum,
    wchar_t* UserID,
    wchar_t* Pwd,
    wchar_t* RemotePath,
    wchar_t* LocalPath,
    FTP_SERVICE_MODE ftpMode,
    CListVersionInfo Version,
    DWORD dwDownloadMaxTime) // OK
{
    this->m_ListManagerInfo.m_DownloaderType = type;
    this->m_ListManagerInfo.m_nPortNum = PortNum;
    this->m_ListManagerInfo.m_ftpMode = ftpMode;
    this->m_ListManagerInfo.m_strLocalPath = LocalPath;
    this->m_ListManagerInfo.m_strRemotePath = RemotePath;
    this->m_ListManagerInfo.m_strPWD = Pwd;
    this->m_ListManagerInfo.m_strServerIP = ServerIP;
    this->m_ListManagerInfo.m_strUserID = UserID;
    this->m_ListManagerInfo.m_Version = Version;
    this->m_ListManagerInfo.m_dwDownloadMaxTime = dwDownloadMaxTime;

    if (GetFileAttributes(LocalPath) == INVALID_FILE_ATTRIBUTES)
        CreateDirectory(LocalPath, 0);

    if (this->m_ListManagerInfo.m_strLocalPath.substr(this->m_ListManagerInfo.m_strLocalPath.size(), 1) != L"\\")
    {
        this->m_ListManagerInfo.m_strLocalPath += L"\\";
    }

    if (this->m_ListManagerInfo.m_strRemotePath.substr(this->m_ListManagerInfo.m_strRemotePath.size(), 1) != L"/")
    {
        this->m_ListManagerInfo.m_strRemotePath += L"/";
    }
}

WZResult		CListManager::LoadScriptList(bool bDonwLoad) // OK
{
    this->m_Result.BuildSuccessResult();

    if (!bDonwLoad && this->IsScriptFileExist())
    {
        goto JUMP;
    }

    this->m_Result = this->FileDownLoad();

    if (!this->m_Result.IsSuccess())
    {
        this->DeleteScriptFiles();
        return this->m_Result;
    }

    if (this->IsScriptFileExist())
    {
    JUMP:
        this->m_Result = this->LoadScript(bDonwLoad);

        if (!this->m_Result.IsSuccess())
            this->DeleteScriptFiles();
    }
    else
    {
        this->DeleteScriptFiles();

        this->m_Result.SetResult(2, 0, L"File Size Zero");
    }

    return this->m_Result;
}

bool			CListManager::IsScriptFileExist() // OK
{
    std::wstring path = this->GetScriptPath();

    for (std::vector<std::wstring>::iterator it = this->m_vScriptFiles.begin(); it != this->m_vScriptFiles.end(); it++)
    {
        std::wstring file_path = path + *(it);

        if (GetFileAttributes(file_path.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            return 0;
        }
    }

    return 1;
}

std::wstring		CListManager::GetScriptPath() // OK
{
    TCHAR buff[MAX_PATH] = { 0 };

    StringCchPrintf(buff, sizeof(buff), L"%03d.%04d.%03d",
        m_ListManagerInfo.m_Version.Zone,
        m_ListManagerInfo.m_Version.year,
        m_ListManagerInfo.m_Version.yearId);

    std::wstring path = this->m_ListManagerInfo.m_strLocalPath;
    path += buff;
    path += L"\\";

    return path;
}

void			CListManager::DeleteScriptFiles() // OK
{
    std::wstring path = this->GetScriptPath();

    for (std::vector<std::wstring>::iterator it = this->m_vScriptFiles.begin(); it != this->m_vScriptFiles.end(); it++)
    {
        std::wstring file_path = path + (*it);

        DeleteFile(file_path.c_str());
    }
}

WZResult		CListManager::FileDownLoad() // OK
{
    if (this->m_ListManagerInfo.m_dwDownloadMaxTime > 0)
    {
        unsigned int ThreadID = 0;

        auto hHandle = (HANDLE)_beginthreadex(0, 0, CListManager::RunFileDownLoadThread, this, 0, &ThreadID);

        if (hHandle == INVALID_HANDLE_VALUE)
        {
            this->m_Result.BuildResult(8, GetLastError(), L"Fail : _beginthreadex");
        }
        else if (WaitForSingleObject(hHandle, this->m_ListManagerInfo.m_dwDownloadMaxTime) == WAIT_TIMEOUT)
        {
            if (this->m_pFTPDownLoader != NULL)
                this->m_pFTPDownLoader->Break();

            WaitForSingleObject(hHandle, INFINITE);

            if (m_pFTPDownLoader != NULL)
                if (m_pFTPDownLoader->GetFileDownloader() != NULL)
                    m_pFTPDownLoader->GetFileDownloader()->Break();

            SAFE_DELETE(m_pFTPDownLoader);

            CloseHandle(hHandle);

            this->m_Result.BuildResult(1, 0, L"Time Out!");
        }
        else
        {
            CloseHandle(hHandle);
        }
    }
    else
    {
        this->m_Result = this->FileDownLoadImpl();
    }

    return this->m_Result;
}

WZResult		CListManager::FileDownLoadImpl() // OK
{
    if (m_pFTPDownLoader != NULL)
    {
        m_pFTPDownLoader->Break();

        if (m_pFTPDownLoader->GetFileDownloader() != NULL)
        {
            m_pFTPDownLoader->GetFileDownloader()->Break();
        }
    }

    SAFE_DELETE(m_pFTPDownLoader); // FIX THIS

    this->m_pFTPDownLoader = new CFTPFileDownLoader;

    this->m_Result = this->m_pFTPDownLoader->DownLoadFiles(
        this->m_ListManagerInfo.m_DownloaderType,
        this->m_ListManagerInfo.m_strServerIP,
        this->m_ListManagerInfo.m_nPortNum,
        this->m_ListManagerInfo.m_strUserID,
        this->m_ListManagerInfo.m_strPWD,
        this->m_ListManagerInfo.m_strRemotePath,
        this->m_ListManagerInfo.m_strLocalPath,
        this->m_ListManagerInfo.m_ftpMode == FTP_MODE_PASSIVE,
        this->m_ListManagerInfo.m_Version,
        this->m_vScriptFiles
    );

    return this->m_Result;
}

unsigned int __stdcall CListManager::RunFileDownLoadThread(LPVOID pParam) // OK
{
    auto* p = reinterpret_cast<CListManager*>(pParam);

    p->m_Result = p->FileDownLoadImpl();

    return 0;
}
#endif