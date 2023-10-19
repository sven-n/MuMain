//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: FileDownloader.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "GameShop\ShopListManager\interface\FileDownloader.h"
#include "HTTPConnecter.h"
#include "FTPConnecter.h"
#include <GameShop\ShopListManager\interface\PathMethod\\Path.h>

#include <process.h>

FileDownloader::FileDownloader(IDownloaderStateEvent* pStateEvent,
    DownloadServerInfo* pServerInfo,
    DownloadFileInfo* pFileInfo) // OK
{
    this->m_bBreak = 0;
    this->m_pStateEvent = pStateEvent;
    this->m_pServerInfo = pServerInfo;
    this->m_pFileInfo = pFileInfo;
    this->m_pConnecter = 0;
    this->m_hLocalFile = INVALID_HANDLE_VALUE;
    this->m_nFileLength = 0;
}

FileDownloader::~FileDownloader() // OK
{
    this->Release();
}

void FileDownloader::Break()
{
    this->m_bBreak = 1;
}

WZResult FileDownloader::DownloadFile() // OK
{
    this->m_bBreak = 0;
    this->m_nFileLength = 0;
    this->Release();
    this->m_pConnecter = this->CreateConnecter();
    this->m_Result = this->m_pConnecter->CreateSession(this->m_hSession);

    if (!this->CanBeContinue())
        goto JUMP_END;

    this->m_Result = this->CreateConnection();
    if (!this->CanBeContinue())
        goto JUMP_END;

    this->m_Result = this->m_pConnecter->OpenRemoteFile(m_hConnection, m_hRemoteFile, m_nFileLength);

    if (!this->CanBeContinue())
        goto JUMP_END;

    this->m_Result = this->CreateLocalFile();

    if (!this->CanBeContinue())
        goto JUMP_END;

    this->m_pFileInfo->SetFileLength(this->m_nFileLength);
    this->m_Result = this->TransferRemoteFile();

JUMP_END:
    return this->m_Result;
}

BOOL FileDownloader::CanBeContinue() // OK
{
    if (this->m_bBreak)
        this->m_Result.SetResult(WZ_USER_BREAK, WZ_SUCCESS, L"[FileDownloader] User Break");
    return this->m_Result.IsSuccess();
}

void FileDownloader::Release() // OK
{
    if (this->m_hLocalFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(this->m_hLocalFile);
        this->m_hLocalFile = INVALID_HANDLE_VALUE;
    }
    if (this->m_hRemoteFile)
    {
        InternetCloseHandle(this->m_hRemoteFile);
        this->m_hRemoteFile = 0;
    }
    if (this->m_hConnection)
    {
        InternetCloseHandle(this->m_hConnection);
        this->m_hConnection = 0;
    }
    if (this->m_hSession)
    {
        InternetCloseHandle(this->m_hSession);
        this->m_hSession = 0;
    }

    SAFE_DELETE(m_pConnecter);
}

IConnecter* FileDownloader::CreateConnecter() // OK
{
    if (m_pServerInfo->GetDownloaderType() == HTTP)
    {
        return new HTTPConnecter(m_pServerInfo, m_pFileInfo);
    }
    else
    {
        return new FTPConnecter(m_pServerInfo, m_pFileInfo);
    }
}

WZResult 			FileDownloader::CreateConnection()
{
    DWORD dwMilliseconds = this->m_pServerInfo->GetConnectTimeout();

    if (dwMilliseconds > 0)
    {
        unsigned int ThreadID = 0;

        auto hHandle = (HANDLE)_beginthreadex(0, 0, FileDownloader::RunConnectThread, this, 0, &ThreadID);

        if (hHandle == INVALID_HANDLE_VALUE)
        {
            this->m_Result.SetResult(DL_BEGIN_THREAD_CONNECTION, GetLastError(), L"[FileDownloader::CreateConnection] Fail : _beginthreadex, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
        }
        else
        {
            if (WaitForSingleObject(hHandle, dwMilliseconds) == WAIT_TIMEOUT)
            {
                InternetCloseHandle(this->m_hSession);
                this->m_hSession = 0;

                WaitForSingleObject(hHandle, INFINITE);

                CloseHandle(hHandle);

                this->m_Result.SetResult(DL_CONNECTION_TIMEOUT, 0, L"[FileDownloader::CreateConnection] Fail : WAIT_TIMEOUT, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
            }
            else
            {
                CloseHandle(hHandle);
            }
        }
    }
    else
    {
        this->m_Result = this->Connection();
    }

    return this->m_Result;
}

unsigned int WINAPI FileDownloader::RunConnectThread(LPVOID pParam)
{
    auto* p = reinterpret_cast<FileDownloader*>(pParam);

    if (p)
    {
        p->m_Result = p->Connection();
    }

    return 0;
}

WZResult 			FileDownloader::Connection()
{
    this->m_Result = this->m_pConnecter->CreateConnection(this->m_hSession, this->m_hConnection);

    return this->m_Result;
}

WZResult 			FileDownloader::TransferRemoteFile()
{
    DWORD CbSize = this->m_pServerInfo->GetReadBufferSize();

    BYTE* buffer = new BYTE[CbSize];

    DWORD TotalSize = 0;
    DWORD ReadSize = 0;

    this->SendStartedDownloadFileEvent(this->m_nFileLength);

    ReadSize = 0;
    this->m_Result = this->m_pConnecter->ReadRemoteFile(this->m_hRemoteFile, buffer, &ReadSize);

    if (this->CanBeContinue())
    {
        while (true)
        {
            if (ReadSize > 0)
            {
                this->m_Result = this->WriteLocalFile(buffer, ReadSize);

                if (!this->CanBeContinue())
                    break;

                TotalSize += ReadSize;
                this->SendProgressDownloadFileEvent(TotalSize);
            }

            if (ReadSize == 0 || this->m_bBreak)
            {
                if (this->CanBeContinue())
                {
                    if (TotalSize >= this->m_nFileLength)
                    {
                        this->m_Result.SetSuccessResult();
                    }
                    else
                    {
                        this->m_Result.SetResult(DL_DIFFERENT_FILE_LENGTH, 0, L"[FileDownloader::TransferRemoteFile] Fail : Different Down File Size, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
                    }
                }

                break;
            }
        }
    }

    this->SendCompletedDownloadFileEvent(this->m_Result);

    delete[] buffer;

    return this->m_Result;
}

WZResult 			FileDownloader::CreateLocalFile()
{
    TCHAR* path = this->m_pFileInfo->GetLocalFilePath();

    if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES || this->m_pServerInfo->IsOverWrite())
    {
        Path::CreateDirectorys(path, 1);

        path = this->m_pFileInfo->GetLocalFilePath();

        DWORD attr = GetFileAttributes(path);
        if ((attr & 1) != 0)
        {
            SetFileAttributes(path, attr & 0xFFFFFFFE);
        }
        this->m_hLocalFile = CreateFile(path, 0x40000000, 0, 0, CREATE_ALWAYS, 0x80, 0);

        if (this->m_hLocalFile == INVALID_HANDLE_VALUE)
        {
            this->m_Result.SetResult(DL_CREATE_LOCALFILE, GetLastError(), L"[FileDownloader::CreateLocalFile] Fail : CreateFile, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
        }
        else
        {
            this->m_Result.SetSuccessResult();
        }
    }
    else
    {
        this->m_Result.SetResult(DL_LOCALFILE_EXISTS, 0, L"[FileDownloader::CreateLocalFile] Fail : Local File Exists, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}

WZResult 			FileDownloader::ReadRemoteFile(BYTE* byReadBuffer, DWORD* dwBytesRead)
{
    if (this->m_hRemoteFile)
    {
        this->m_Result = this->m_pConnecter->ReadRemoteFile(this->m_hRemoteFile, byReadBuffer, dwBytesRead);
    }
    else
    {
        this->m_Result.SetResult(DL_READ_REMOTEFILE, 0, L"[FileDownloader::ReadRemoteFile] Fail : ReadRemoteFile, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}

WZResult 			FileDownloader::WriteLocalFile(BYTE* byReadBuffer, DWORD dwBytesRead)
{
    DWORD NumberOfBytesWritten = 0;

    if (WriteFile(this->m_hLocalFile, byReadBuffer, dwBytesRead, &NumberOfBytesWritten, 0) && dwBytesRead == NumberOfBytesWritten)
    {
        this->m_Result.SetSuccessResult();
    }
    else
    {
        this->m_Result.SetResult(DL_WRITE_LOCALFILE, GetLastError(), L"[FileDownloader::WriteLocalFile] Fail : WriteFile, FileName = %s", this->m_pFileInfo->GetRemoteFilePath());
    }

    return this->m_Result;
}

void				FileDownloader::SendStartedDownloadFileEvent(ULONGLONG nFileLength)
{
    if (this->m_pStateEvent != NULL)
    {
        this->m_pStateEvent->OnStartedDownloadFile(this->m_pFileInfo->GetFileName(), nFileLength);
    }
}

void				FileDownloader::SendCompletedDownloadFileEvent(WZResult wzResult)
{
    if (this->m_pStateEvent != NULL)
    {
        this->m_pStateEvent->OnCompletedDownloadFile(this->m_pFileInfo->GetFileName(), wzResult);
    }
}

void				FileDownloader::SendProgressDownloadFileEvent(ULONGLONG nTotalBytesRead)
{
    if (this->m_pStateEvent != NULL)
    {
        this->m_pStateEvent->OnProgressDownloadFile(this->m_pFileInfo->GetFileName(), nTotalBytesRead);
    }
}

#endif