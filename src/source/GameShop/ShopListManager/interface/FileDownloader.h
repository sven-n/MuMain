/*******************************************************************************
 *	FileDownloader — libcurl-based file download engine
 *	Migrated from WinINet (Story 7.6.6)
 *******************************************************************************/

#pragma once

#include <cstdint>
#include <fstream>
#include <curl/curl.h>
#include "GameShop/ShopListManager/interface/IConnecter.h"
#include "GameShop/ShopListManager/interface/IDownloaderStateEvent.h"

class FileDownloader
{
public:
    FileDownloader(IDownloaderStateEvent* pStateEvent, DownloadServerInfo* pServerInfo, DownloadFileInfo* pFileInfo);
    ~FileDownloader();

    void Break();
    WZResult DownloadFile();

private:
    bool CanBeContinue();
    void Release();

    IConnecter* CreateConnecter();

    static size_t CurlWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
    static int CurlProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal,
                                    curl_off_t ulnow);

    void SendStartedDownloadFileEvent(uint64_t nFileLength);
    void SendCompletedDownloadFileEvent(WZResult wzResult);
    void SendProgressDownloadFileEvent(uint64_t nTotalBytesRead);

    volatile bool m_bBreak;
    WZResult m_Result;

    IDownloaderStateEvent* m_pStateEvent;
    DownloadServerInfo* m_pServerInfo;
    DownloadFileInfo* m_pFileInfo;
    IConnecter* m_pConnecter;

    std::ofstream m_LocalFile;
    uint64_t m_nFileLength;
};
