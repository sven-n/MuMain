//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: HTTPConnecter.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "HTTPConnecter.h"
#include "GameShop\ShopListManager\interface\PathMethod\Path.h"

HTTPConnecter::HTTPConnecter(DownloadServerInfo* pServerInfo, DownloadFileInfo* pFileInfo) : IConnecter(pServerInfo, pFileInfo)
{
}

HTTPConnecter::~HTTPConnecter()
{
}

WZResult		HTTPConnecter::CreateSession(HINTERNET& hSession)
{
    wchar_t path[MAX_PATH] = { 0 };

    Path::GetCurrentFileName(path);

    if ((hSession = InternetOpen(path, 0, 0, 0, 0)))
    {
        this->m_Result.SetSuccessResult();
    }
    else
    {
        this->m_Result.SetResult(DL_CREATE_SESSION, GetLastError(), L"[HTTPConnecter::CreateSession] Fail : InternetOpen, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}

WZResult		HTTPConnecter::CreateConnection(HINTERNET& hSession,
    HINTERNET& hConnection)
{
    this->m_pServerInfo->GetPassword();

    if ((hConnection = InternetConnect(hSession,
        this->m_pServerInfo->GetServerURL(),
        this->m_pServerInfo->GetPort(),
        this->m_pServerInfo->GetUserID(),
        this->m_pServerInfo->GetPassword(),
        3,
        0,
        (DWORD_PTR)this
    )))
    {
        this->m_Result.SetSuccessResult();
    }
    else
    {
        this->m_Result.SetResult(DL_CREATE_CONNECTION, GetLastError(), L"[HTTPConnecter::CreateConnection] Fail : InternetConnect, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}

WZResult		HTTPConnecter::OpenRemoteFile(HINTERNET& hConnection,
    HINTERNET& hRemoteFile,
    ULONGLONG& nFileLength)
{
    hRemoteFile = HttpOpenRequest(hConnection, L"GET", this->m_pFileInfo->GetRemoteFilePath(), L"HTTP/1.0", 0, 0, 0x80000000, 0);

    if (hRemoteFile)
    {
        HttpSendRequest(hRemoteFile, 0, 0, 0, 0);

        wchar_t buffer[32];

        memset(buffer, 0, sizeof(buffer));

        DWORD dwBufferLength = 32;

        if (HttpQueryInfo(hRemoteFile, HTTP_QUERY_STATUS_CODE, buffer, &dwBufferLength, 0))
        {
            if (_wtoi64(buffer) == HTTP_STATUS_OK)
            {
                memset(buffer, 0, sizeof(buffer));
                dwBufferLength = 32;

                if (HttpQueryInfo(hRemoteFile, HTTP_QUERY_CONTENT_LENGTH, buffer, &dwBufferLength, 0))
                {
                    nFileLength = _wtoi64(buffer);
                    this->m_Result.SetSuccessResult();
                }
                else
                {
                    this->m_Result.SetResult(DL_GET_FILE_LENGTH, GetLastError(), L"[HTTPConnecter::OpenRemoteFile] Fail : HttpQueryInfo - HTTP_QUERY_CONTENT_LENGTH, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
                }
            }
            else
            {
                this->m_Result.SetResult(DL_HTTP_STATUS_NOT_OK, 0, L"[HTTPConnecter::OpenRemoteFile] Fail : Not HTTP_STATUS_OK, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
            }
        }
        else
        {
            this->m_Result.SetResult(DL_HTTP_QUERY_INFO, GetLastError(), L"[HTTPConnecter::OpenRemoteFile] Fail : HttpQueryInfo - HTTP_QUERY_STATUS_CODE, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
        }
    }
    else
    {
        this->m_Result.SetResult(DL_OPEN_REMOTEFILE, GetLastError(), L"[HTTPConnecter::OpenRemoteFile] Fail : HttpOpenRequest, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}

WZResult		HTTPConnecter::ReadRemoteFile(HINTERNET& hRemoteFile,
    BYTE* byReadBuffer,
    DWORD* dwBytesRead)
{
    DWORD dwNumberOfBytesAvailable = 0;

    InternetQueryDataAvailable(hRemoteFile, &dwNumberOfBytesAvailable, 0, 0);

    DWORD Size = this->m_pServerInfo->GetReadBufferSize();

    if (InternetReadFile(hRemoteFile, byReadBuffer, Size, dwBytesRead))
    {
        this->m_Result.SetSuccessResult();
    }
    else
    {
        this->m_Result.SetResult(DL_READ_REMOTEFILE, GetLastError(), L"[HTTPConnecter::ReadRemoteFile] Fail : InternetReadFile, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}
#endif