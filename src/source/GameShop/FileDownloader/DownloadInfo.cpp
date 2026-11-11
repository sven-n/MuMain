//************************************************************************
//
// FILE: DownloadInfo.cpp
// Migrated to portable types (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "Include.h"

DownloadFileInfo::DownloadFileInfo()
{
    this->m_uFileLength = 0;
    memset(this->m_szFileName, 0, sizeof(this->m_szFileName));
    memset(this->m_szLocalFilePath, 0, sizeof(this->m_szLocalFilePath));
    memset(this->m_szRemoteFilePath, 0, sizeof(this->m_szRemoteFilePath));
    memset(this->m_szTargerDirPath, 0, sizeof(this->m_szTargerDirPath));
}

DownloadFileInfo::~DownloadFileInfo() {}

wchar_t* DownloadFileInfo::GetFileName()
{
    return m_szFileName;
}

wchar_t* DownloadFileInfo::GetLocalFilePath()
{
    return m_szLocalFilePath;
}

wchar_t* DownloadFileInfo::GetRemoteFilePath()
{
    return m_szRemoteFilePath;
}

wchar_t* DownloadFileInfo::GetTargetDirPath()
{
    return m_szTargerDirPath;
}

uint64_t DownloadFileInfo::GetFileLength()
{
    return this->m_uFileLength;
}

void DownloadFileInfo::SetFilePath(wchar_t* szFileName, wchar_t* szLocalFilePath, wchar_t* szRemoteFilePath,
                                   wchar_t* szTargerDirPath)
{
    wcsncpy(this->m_szFileName, szFileName, MAX_PATH - 1);
    this->m_szFileName[MAX_PATH - 1] = L'\0';
    wcsncpy(this->m_szLocalFilePath, szLocalFilePath, MAX_PATH - 1);
    this->m_szLocalFilePath[MAX_PATH - 1] = L'\0';
    wcsncpy(this->m_szRemoteFilePath, szRemoteFilePath, DL_MAX_URL_LENGTH - 1);
    this->m_szRemoteFilePath[DL_MAX_URL_LENGTH - 1] = L'\0';
    if (szTargerDirPath)
    {
        wcsncpy(this->m_szTargerDirPath, szTargerDirPath, MAX_PATH - 1);
        this->m_szTargerDirPath[MAX_PATH - 1] = L'\0';
    }
}

void DownloadFileInfo::SetFileLength(uint64_t uFileLength)
{
    this->m_uFileLength = uFileLength;
}

DownloadServerInfo::DownloadServerInfo()
{
    this->m_nPort = 21;
    this->m_DownloaderType = FTP;
    this->m_dwReadBufferSize = DL_DEFAULT_BUFFER_SIZE;
    this->m_bOverWrite = true;
    this->m_bPassive = false;
    this->m_dwConnectTimeout = 0;
    memset(this->m_szServerURL, 0, sizeof(this->m_szServerURL));
    memset(this->m_szUserID, 0, sizeof(this->m_szUserID));
    memset(this->m_szPassword, 0, sizeof(this->m_szPassword));
}

DownloadServerInfo::~DownloadServerInfo() {}

uint32_t DownloadServerInfo::GetConnectTimeout()
{
    return this->m_dwConnectTimeout;
}

DownloaderType DownloadServerInfo::GetDownloaderType()
{
    return this->m_DownloaderType;
}

wchar_t* DownloadServerInfo::GetPassword()
{
    return this->m_szPassword;
}

unsigned short DownloadServerInfo::GetPort()
{
    return this->m_nPort;
}

uint32_t DownloadServerInfo::GetReadBufferSize()
{
    return this->m_dwReadBufferSize;
}

wchar_t* DownloadServerInfo::GetServerURL()
{
    return this->m_szServerURL;
}

wchar_t* DownloadServerInfo::GetUserID()
{
    return this->m_szUserID;
}

bool DownloadServerInfo::IsOverWrite()
{
    return this->m_bOverWrite;
}

bool DownloadServerInfo::IsPassive()
{
    return this->m_bPassive;
}

void DownloadServerInfo::SetServerInfo(wchar_t* szServerURL, unsigned short nPort, wchar_t* szUserID,
                                       wchar_t* szPassword)
{
    auto* search = wcschr(szServerURL, L':');
    if (search && search[1] == L'/' && search[2] == L'/')
    {
        wcsncpy(this->m_szServerURL, search + 3, DL_MAX_URL_LENGTH - 1);
    }
    else
    {
        wcsncpy(this->m_szServerURL, szServerURL, DL_MAX_URL_LENGTH - 1);
    }
    this->m_szServerURL[DL_MAX_URL_LENGTH - 1] = L'\0';
    this->m_nPort = nPort;
    wcsncpy(this->m_szUserID, szUserID, DL_MAX_USER_NAME_LENGTH - 1);
    this->m_szUserID[DL_MAX_USER_NAME_LENGTH - 1] = L'\0';
    wcsncpy(this->m_szPassword, szPassword, DL_MAX_PASSWORD_LENGTH - 1);
    this->m_szPassword[DL_MAX_PASSWORD_LENGTH - 1] = L'\0';
}

void DownloadServerInfo::SetDownloaderType(DownloaderType dwDownloaderType)
{
    this->m_DownloaderType = dwDownloaderType;
}

void DownloadServerInfo::SetReadBufferSize(uint32_t dwReadBufferSize)
{
    this->m_dwReadBufferSize = dwReadBufferSize;
}

void DownloadServerInfo::SetOverWrite(bool bOverWrite)
{
    this->m_bOverWrite = bOverWrite;
}

void DownloadServerInfo::SetPassiveMode(bool bPassive)
{
    this->m_bPassive = bPassive;
}

void DownloadServerInfo::SetConnectTimeout(uint32_t dwConnectTimeout)
{
    this->m_dwConnectTimeout = dwConnectTimeout;
}
#endif
