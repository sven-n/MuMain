/*******************************************************************************
 *	Download info classes — portable types
 *	Migrated from WinINet types (Story 7.6.6)
 *******************************************************************************/

#pragma once

#include <cstdint>
#include <cwchar>

#define DL_DEFAULT_BUFFER_SIZE 4096

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define DL_MAX_URL_LENGTH 2084
#define DL_MAX_USER_NAME_LENGTH 256
#define DL_MAX_PASSWORD_LENGTH 256

typedef enum _DownloaderType
{
    FTP,
    HTTP,
} DownloaderType;

class DownloadFileInfo
{
public:
    DownloadFileInfo();
    ~DownloadFileInfo();

    wchar_t* GetFileName();
    wchar_t* GetLocalFilePath();
    wchar_t* GetRemoteFilePath();
    wchar_t* GetTargetDirPath();
    uint64_t GetFileLength();

    void SetFilePath(wchar_t* szFileName, wchar_t* szLocalFilePath, wchar_t* szRemoteFilePath,
                     wchar_t* szTargerDirPath);
    void SetFileLength(uint64_t uFileLength);

private:
    wchar_t m_szFileName[MAX_PATH];
    wchar_t m_szLocalFilePath[MAX_PATH];
    wchar_t m_szRemoteFilePath[DL_MAX_URL_LENGTH];
    wchar_t m_szTargerDirPath[MAX_PATH];
    uint64_t m_uFileLength;
};

class DownloadServerInfo
{
public:
    DownloadServerInfo();
    ~DownloadServerInfo();

    wchar_t* GetServerURL();
    wchar_t* GetUserID();
    wchar_t* GetPassword();
    unsigned short GetPort();
    DownloaderType GetDownloaderType();
    uint32_t GetReadBufferSize();
    uint32_t GetConnectTimeout();
    bool IsOverWrite();
    bool IsPassive();

    void SetServerInfo(wchar_t* szServerURL, unsigned short nPort, wchar_t* szUserID, wchar_t* szPassword);
    void SetDownloaderType(DownloaderType dwDownloaderType);
    void SetReadBufferSize(uint32_t dwReadBufferSize);
    void SetOverWrite(bool bOverWrite);
    void SetPassiveMode(bool bPassive);
    void SetConnectTimeout(uint32_t dwConnectTimeout);

private:
    wchar_t m_szServerURL[DL_MAX_URL_LENGTH];
    wchar_t m_szUserID[DL_MAX_USER_NAME_LENGTH];
    wchar_t m_szPassword[DL_MAX_PASSWORD_LENGTH];
    unsigned short m_nPort;
    DownloaderType m_DownloaderType;
    uint32_t m_dwReadBufferSize;
    bool m_bOverWrite;
    bool m_bPassive;
    uint32_t m_dwConnectTimeout;
};
