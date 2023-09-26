//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: DownloadInfo.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "Include.h"

DownloadFileInfo::DownloadFileInfo() // OK
{
    this->m_uFileLength = 0;
    RtlSecureZeroMemory(this->m_szFileName, sizeof(this->m_szFileName));
    RtlSecureZeroMemory(this->m_szLocalFilePath, sizeof(this->m_szLocalFilePath));
    RtlSecureZeroMemory(this->m_szRemoteFilePath, sizeof(this->m_szRemoteFilePath));
    RtlSecureZeroMemory(this->m_szTargerDirPath, sizeof(this->m_szTargerDirPath));
}

DownloadFileInfo::~DownloadFileInfo() // OK
{
}

TCHAR* DownloadFileInfo::GetFileName() // OK
{
    return m_szFileName;
}

TCHAR* DownloadFileInfo::GetLocalFilePath() // OK
{
    return m_szLocalFilePath;
}

TCHAR* DownloadFileInfo::GetRemoteFilePath() // OK
{
    return m_szRemoteFilePath;
}

TCHAR* DownloadFileInfo::GetTargetDirPath() // OK
{
    return m_szTargerDirPath;
}

ULONGLONG DownloadFileInfo::GetFileLength() // OK
{
    return this->m_uFileLength;
}

void DownloadFileInfo::SetFilePath(TCHAR* szFileName, TCHAR* szLocalFilePath, TCHAR* szRemoteFilePath, TCHAR* szTargerDirPath) // OK
{
    StringCchCopy(this->m_szFileName, sizeof(this->m_szFileName), szFileName);
    StringCchCopy(this->m_szLocalFilePath, sizeof(this->m_szLocalFilePath), szLocalFilePath);
    StringCchCopy(this->m_szRemoteFilePath, sizeof(this->m_szRemoteFilePath), szRemoteFilePath);
    if (szTargerDirPath)
        StringCchCopy(this->m_szTargerDirPath, sizeof(this->m_szTargerDirPath), szTargerDirPath);
}

void  DownloadFileInfo::SetFileLength(ULONGLONG uFileLength) // OK
{
    this->m_uFileLength = uFileLength;
}

DownloadServerInfo::DownloadServerInfo() // OK
{
    this->m_nPort = 21;
    this->m_DownloaderType = FTP;
    this->m_dwReadBufferSize = DL_DEFAULT_BUFFER_SIZE;
    this->m_bOverWrite = 1;
    this->m_bPassive = 0;
    this->m_dwConnectTimeout = 0;
    RtlSecureZeroMemory(this->m_szServerURL, sizeof(this->m_szServerURL));
    RtlSecureZeroMemory(this->m_szUserID, sizeof(this->m_szUserID));
    RtlSecureZeroMemory(this->m_szPassword, sizeof(this->m_szPassword));
}

DownloadServerInfo::~DownloadServerInfo() // OK
{
}

DWORD DownloadServerInfo::GetConnectTimeout() // OK
{
    return this->m_dwConnectTimeout;
}

DownloaderType DownloadServerInfo::GetDownloaderType() // OK
{
    return this->m_DownloaderType;
}

TCHAR* DownloadServerInfo::GetPassword() // OK
{
    return this->m_szPassword;
}

INTERNET_PORT DownloadServerInfo::GetPort() // OK
{
    return this->m_nPort;
}

DWORD DownloadServerInfo::GetReadBufferSize() // OK
{
    return this->m_dwReadBufferSize;
}

TCHAR* DownloadServerInfo::GetServerURL() // OK
{
    return this->m_szServerURL;
}

TCHAR* DownloadServerInfo::GetUserID() // OK
{
    return this->m_szUserID;
}

BOOL DownloadServerInfo::IsOverWrite() // OK
{
    return this->m_bOverWrite;
}

BOOL DownloadServerInfo::IsPassive() // OK
{
    return this->m_bPassive;
}

void DownloadServerInfo::SetServerInfo(TCHAR* szServerURL, INTERNET_PORT nPort, TCHAR* szUserID, TCHAR* szPassword) // OK
{
    auto* search = wcschr(szServerURL, ':');
    if (search && search[1] == '/' && search[2] == '/')
        StringCchCopy(this->m_szServerURL, sizeof(this->m_szServerURL), search + 3);
    else
        StringCchCopy(this->m_szServerURL, sizeof(this->m_szServerURL), szServerURL);
    this->m_nPort = nPort;
    StringCchCopy(this->m_szUserID, sizeof(this->m_szUserID), szUserID);
    StringCchCopy(this->m_szPassword, sizeof(this->m_szPassword), szPassword);
}

void DownloadServerInfo::SetDownloaderType(DownloaderType dwDownloaderType) // OK
{
    this->m_DownloaderType = dwDownloaderType;
}

void DownloadServerInfo::SetReadBufferSize(DWORD dwReadBufferSize) // OK
{
    this->m_dwReadBufferSize = dwReadBufferSize;
}

void DownloadServerInfo::SetOverWrite(BOOL bOverWrite) // OK
{
    this->m_bOverWrite = bOverWrite;
}

void DownloadServerInfo::SetPassiveMode(BOOL bPassive) // OK
{
    this->m_bPassive = bPassive;
}

void DownloadServerInfo::SetConnectTimeout(DWORD dwConnectTimeout) // OK
{
    this->m_dwConnectTimeout = dwConnectTimeout;
}
#endif