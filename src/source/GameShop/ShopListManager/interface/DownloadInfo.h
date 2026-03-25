/*******************************************************************************
 *	�� �� �� : ������
 *	�� �� �� : 2009.06.10
 *	��    �� : Download�� �ʿ��� ���� ����
 *******************************************************************************/

#pragma once

#ifdef _WIN32
#include <wininet.h>
#endif
#define DL_DEFAULT_BUFFER_SIZE 4096

typedef enum _DownloaderType
{
    FTP,
    HTTP,
} DownloaderType;

class DownloadFileInfo
{
public:
    // Constructor, Destructor

    DownloadFileInfo();
    ~DownloadFileInfo();

    // Get Function
    //			���� ��� ��������
    TCHAR* GetFileName();
    TCHAR* GetLocalFilePath();
    TCHAR* GetRemoteFilePath();
    TCHAR* GetTargetDirPath();
    ULONGLONG GetFileLength();
    // Set Function
    //		���� ��� ����
    void SetFilePath(TCHAR* szFileName, TCHAR* szLocalFilePath, TCHAR* szRemoteFilePath, TCHAR* szTargerDirPath);
    void SetFileLength(ULONGLONG uFileLength);

private:
    // Member Object

    //			���� �̸�
    TCHAR m_szFileName[MAX_PATH];
    //			���� ��ü ���
    TCHAR m_szLocalFilePath[MAX_PATH];
    //			����Ʈ ��ü ���
    TCHAR m_szRemoteFilePath[INTERNET_MAX_URL_LENGTH];
    //			��ġ Ǯ� ���� ���
    TCHAR m_szTargerDirPath[MAX_PATH];
    //			���� ������
    ULONGLONG m_uFileLength;
};

class DownloadServerInfo
{
public:
    // Constructor, Destructor

    DownloadServerInfo();
    ~DownloadServerInfo();

    // Get Function

    //				���� �ּ� ��������
    TCHAR* GetServerURL();
    //				���� ���� ���� ��������
    TCHAR* GetUserID();
    //				���� ���� ���� ��� ��������
    TCHAR* GetPassword();
    //				��Ʈ ��ȣ ��������
    INTERNET_PORT GetPort();
    //				Ÿ��ε� Ÿ�� ��������
    DownloaderType GetDownloaderType();
    //				���� ������ ��������
    DWORD GetReadBufferSize();
    //				Ŀ��Ʈ Ÿ�Ӿƿ� ��������
    DWORD GetConnectTimeout();
    //				����� ����
    BOOL IsOverWrite();
    //				�нú� ��� ����
    BOOL IsPassive();

    // Set Function

    //				���� ���� ����
    void SetServerInfo(TCHAR* szServerURL, INTERNET_PORT nPort, TCHAR* szUserID, TCHAR* szPassword);
    //				�ٿ�ε� Ÿ�� ����
    void SetDownloaderType(DownloaderType dwDownloaderType);
    //				���� ������ ����
    void SetReadBufferSize(DWORD dwReadBufferSize);
    //				���� ���� ���� �� ����� ����
    void SetOverWrite(BOOL bOverWrite);
    //				�нú� ��� ����
    void SetPassiveMode(BOOL bPassive);
    //				Ŀ��Ʈ Ÿ�Ӿƿ� ����
    void SetConnectTimeout(DWORD dwConnectTimeout);

private:
    // Member Object

    // 							Server �ּ�
    TCHAR m_szServerURL[INTERNET_MAX_URL_LENGTH];
    // 							���� ���� ��
    TCHAR m_szUserID[INTERNET_MAX_USER_NAME_LENGTH];
    // 							���� ���� Password
    TCHAR m_szPassword[INTERNET_MAX_PASSWORD_LENGTH];
    // 							���� ��Ʈ default = INTERNET_DEFAULT_FTP_PORT (21)
    INTERNET_PORT m_nPort;
    //							�ٿ�δ� Ÿ�� - ��������
    DownloaderType m_DownloaderType;
    // 							�ٿ�ε� ��Ŷ ������ ���� default = 4096
    DWORD m_dwReadBufferSize;
    //							Local File ������ ��� ����� ���� default = TRUE
    BOOL m_bOverWrite;
    //							Passive ���� default = FALSE
    BOOL m_bPassive;
    //							Ŀ��Ʈ Ÿ�Ӿƿ�
    DWORD m_dwConnectTimeout;
};
