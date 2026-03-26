//************************************************************************
//
// FILE: ListManager.cpp
// Migrated to std::thread + std::filesystem (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ListManager.h"

#include <thread>
#include <future>
#include <filesystem>
#include <cwchar>

// cppcheck-suppress uninitMemberVar
CListManager::CListManager()
{
    this->m_pFTPDownLoader = nullptr;
    this->m_ListManagerInfo.m_strServerIP.clear();
    this->m_ListManagerInfo.m_strUserID.clear();
    this->m_ListManagerInfo.m_strPWD.clear();
    this->m_ListManagerInfo.m_strRemotePath.clear();
    this->m_ListManagerInfo.m_strLocalPath.clear();
    this->m_vScriptFiles.clear();
}

CListManager::~CListManager()
{
    SAFE_DELETE(m_pFTPDownLoader);
}

void CListManager::SetListManagerInfo(DownloaderType type, const wchar_t* ServerIP, const wchar_t* UserID,
                                      const wchar_t* Pwd, const wchar_t* RemotePath, const wchar_t* LocalPath,
                                      CListVersionInfo Version, uint32_t dwDownloadMaxTime)
{
    unsigned short port = 80;

    if (type == FTP)
        port = 21;

    this->SetListManagerInfo(type, ServerIP, port, UserID, Pwd, RemotePath, LocalPath, FTP_MODE_ACTIVE, Version,
                             dwDownloadMaxTime);
}

void CListManager::SetListManagerInfo(DownloaderType type, const wchar_t* ServerIP, unsigned short PortNum,
                                      const wchar_t* UserID, const wchar_t* Pwd, const wchar_t* RemotePath,
                                      const wchar_t* LocalPath, FTP_SERVICE_MODE ftpMode, CListVersionInfo Version,
                                      uint32_t dwDownloadMaxTime)
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

    std::error_code ec;
    if (!std::filesystem::exists(std::filesystem::path(LocalPath), ec))
        std::filesystem::create_directory(std::filesystem::path(LocalPath), ec);

    if (!this->m_ListManagerInfo.m_strLocalPath.empty() && this->m_ListManagerInfo.m_strLocalPath.back() != L'/')
    {
        this->m_ListManagerInfo.m_strLocalPath += L"/";
    }

    if (!this->m_ListManagerInfo.m_strRemotePath.empty() && this->m_ListManagerInfo.m_strRemotePath.back() != L'/')
    {
        this->m_ListManagerInfo.m_strRemotePath += L"/";
    }
}

WZResult CListManager::LoadScriptList(bool bDonwLoad)
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

bool CListManager::IsScriptFileExist()
{
    std::wstring path = this->GetScriptPath();

    for (std::vector<std::wstring>::iterator it = this->m_vScriptFiles.begin(); it != this->m_vScriptFiles.end(); it++)
    {
        std::wstring file_path = path + *(it);

        std::error_code ec;
        if (!std::filesystem::exists(std::filesystem::path(file_path), ec))
        {
            return false;
        }
    }

    return true;
}

std::wstring CListManager::GetScriptPath()
{
    wchar_t buff[MAX_PATH] = {0};

    swprintf(buff, MAX_PATH, L"%03d.%04d.%03d", m_ListManagerInfo.m_Version.Zone, m_ListManagerInfo.m_Version.year,
             m_ListManagerInfo.m_Version.yearId);

    std::wstring path = this->m_ListManagerInfo.m_strLocalPath;
    path += buff;
    path += L"/";

    return path;
}

void CListManager::DeleteScriptFiles()
{
    std::wstring path = this->GetScriptPath();

    for (std::vector<std::wstring>::iterator it = this->m_vScriptFiles.begin(); it != this->m_vScriptFiles.end(); it++)
    {
        std::wstring file_path = path + (*it);

        std::error_code ec;
        std::filesystem::remove(std::filesystem::path(file_path), ec);
    }
}

WZResult CListManager::FileDownLoad()
{
    if (this->m_ListManagerInfo.m_dwDownloadMaxTime > 0)
    {
        auto future = std::async(std::launch::async, [this]() { return this->FileDownLoadImpl(); });

        auto status = future.wait_for(std::chrono::milliseconds(this->m_ListManagerInfo.m_dwDownloadMaxTime));
        if (status == std::future_status::timeout)
        {
            if (this->m_pFTPDownLoader != nullptr)
                this->m_pFTPDownLoader->Break();

            // Wait for the thread to finish after breaking
            this->m_Result = future.get();

            if (m_pFTPDownLoader != nullptr)
                if (m_pFTPDownLoader->GetFileDownloader() != nullptr)
                    m_pFTPDownLoader->GetFileDownloader()->Break();

            SAFE_DELETE(m_pFTPDownLoader);

            this->m_Result.BuildResult(1, 0, L"Time Out!");
        }
        else
        {
            this->m_Result = future.get();
        }
    }
    else
    {
        this->m_Result = this->FileDownLoadImpl();
    }

    return this->m_Result;
}

WZResult CListManager::FileDownLoadImpl()
{
    if (m_pFTPDownLoader != nullptr)
    {
        m_pFTPDownLoader->Break();

        if (m_pFTPDownLoader->GetFileDownloader() != nullptr)
        {
            m_pFTPDownLoader->GetFileDownloader()->Break();
        }
    }

    SAFE_DELETE(m_pFTPDownLoader);

    this->m_pFTPDownLoader = new CFTPFileDownLoader;

    this->m_Result = this->m_pFTPDownLoader->DownLoadFiles(
        this->m_ListManagerInfo.m_DownloaderType, this->m_ListManagerInfo.m_strServerIP,
        this->m_ListManagerInfo.m_nPortNum, this->m_ListManagerInfo.m_strUserID, this->m_ListManagerInfo.m_strPWD,
        this->m_ListManagerInfo.m_strRemotePath, this->m_ListManagerInfo.m_strLocalPath,
        this->m_ListManagerInfo.m_ftpMode == FTP_MODE_PASSIVE, this->m_ListManagerInfo.m_Version, this->m_vScriptFiles);

    return this->m_Result;
}
#endif
