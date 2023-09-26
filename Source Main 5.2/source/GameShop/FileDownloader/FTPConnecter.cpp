//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: FTPConnecter.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "FTPConnecter.h"
#include "GameShop\ShopListManager\interface\PathMethod\Path.h"

FTPConnecter::FTPConnecter(DownloadServerInfo* pServerInfo, DownloadFileInfo* pFileInfo) : IConnecter(pServerInfo, pFileInfo)
{
}

FTPConnecter::~FTPConnecter()
{
}

WZResult		FTPConnecter::CreateSession(HINTERNET& hSession)
{
    wchar_t path[MAX_PATH] = { 0 };

    Path::GetCurrentFileName(path);

    if ((hSession = InternetOpen(path, 0, 0, 0, 0)))
    {
        this->m_Result.SetSuccessResult();
    }
    else
    {
        this->m_Result.SetResult(DL_CREATE_SESSION, GetLastError(), L"[FTPConnecter::CreateSession] Fail : InternetOpen, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}

WZResult		FTPConnecter::CreateConnection(HINTERNET& hSession,
    HINTERNET& hConnection)
{
    if ((hConnection = InternetConnect(hSession,
        this->m_pServerInfo->GetServerURL(),
        this->m_pServerInfo->GetPort(),
        this->m_pServerInfo->GetUserID(),
        this->m_pServerInfo->GetPassword(),
        1,
        this->m_pServerInfo->IsPassive() != 0 ? 0x8000000 : 0,
        (DWORD_PTR)this
    )))
    {
        this->m_Result.SetSuccessResult();
    }
    else
    {
        this->m_Result.SetResult(DL_CREATE_CONNECTION, GetLastError(), L"[FTPConnecter::CreateConnection] Fail : InternetConnect, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}

WZResult		FTPConnecter::OpenRemoteFile(HINTERNET& hConnection,
    HINTERNET& hRemoteFile,
    ULONGLONG& nFileLength)
{
    struct _WIN32_FIND_DATAW FindFileData = { 0 };

    HINTERNET hInternet = FtpFindFirstFile(hConnection, this->m_pFileInfo->GetRemoteFilePath(), &FindFileData, 0x84000000, (DWORD_PTR)this);

    if (hInternet)
    {
        nFileLength = (ULONGLONG)((ULONGLONG)FindFileData.nFileSizeHigh << (ULONGLONG)32) | (ULONGLONG)(FindFileData.nFileSizeLow);

        InternetCloseHandle(hInternet);

        hRemoteFile = FtpOpenFile(hConnection, this->m_pFileInfo->GetRemoteFilePath(), 0x80000000, 0x84000002, (DWORD_PTR)this);

        if (hRemoteFile)
        {
            this->m_Result.SetSuccessResult();
        }
        else
        {
            this->m_Result.SetResult(DL_OPEN_REMOTEFILE, GetLastError(), L"[FTPConnecter::OpenRemoteFile] Fail : FtpOpenFile, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
        }
    }
    else
    {
        this->m_Result.SetResult(DL_GET_FILE_LENGTH, 0, L"[FTPConnecter::OpenRemoteFile] Fail : FtpFindFirstFile, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}

WZResult		FTPConnecter::ReadRemoteFile(HINTERNET& hRemoteFile,
    BYTE* byReadBuffer,
    DWORD* dwBytesRead)
{
    DWORD Size = this->m_pServerInfo->GetReadBufferSize();

    if (InternetReadFile(hRemoteFile, byReadBuffer, Size, dwBytesRead))
    {
        this->m_Result.SetSuccessResult();
    }
    else
    {
        this->m_Result.SetResult(DL_READ_REMOTEFILE, GetLastError(), L"[FTPConnecter::ReadRemoteFile] Fail : InternetReadFile, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}
#endif