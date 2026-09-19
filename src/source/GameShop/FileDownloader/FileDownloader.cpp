//************************************************************************
//
// FILE: FileDownloader.cpp
// Migrated from WinINet to libcurl (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "GameShop/ShopListManager/interface/FileDownloader.h"
#include "HTTPConnecter.h"
#include "FTPConnecter.h"
#include <GameShop/ShopListManager/interface/PathMethod/Path.h>

#include <thread>
#include <future>
#include <filesystem>

FileDownloader::FileDownloader(IDownloaderStateEvent* pStateEvent, DownloadServerInfo* pServerInfo,
                               DownloadFileInfo* pFileInfo)
{
    this->m_bBreak = false;
    this->m_pStateEvent = pStateEvent;
    this->m_pServerInfo = pServerInfo;
    this->m_pFileInfo = pFileInfo;
    this->m_pConnecter = nullptr;
    this->m_nFileLength = 0;
}

FileDownloader::~FileDownloader()
{
    this->Release();
}

void FileDownloader::Break()
{
    this->m_bBreak = true;
}

size_t FileDownloader::CurlWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    auto* stream = static_cast<std::ofstream*>(userdata);
    size_t bytes = size * nmemb;
    stream->write(static_cast<const char*>(ptr), static_cast<std::streamsize>(bytes));
    return stream->good() ? bytes : 0;
}

int FileDownloader::CurlProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t /*ultotal*/,
                                         curl_off_t /*ulnow*/)
{
    auto* downloader = static_cast<FileDownloader*>(clientp);

    if (downloader->m_bBreak)
        return 1; // Abort transfer

    if (dlnow > 0)
        downloader->SendProgressDownloadFileEvent(static_cast<uint64_t>(dlnow));

    return 0;
}

WZResult FileDownloader::DownloadFile()
{
    this->m_bBreak = false;
    this->m_nFileLength = 0;
    this->Release();

    this->m_pConnecter = this->CreateConnecter();

    // Build URL from connecter
    std::string url = this->m_pConnecter->BuildURL();

    // Create local directory structure
    wchar_t* localPath = this->m_pFileInfo->GetLocalFilePath();

    if (!this->m_pServerInfo->IsOverWrite())
    {
        // Check if file already exists
        std::error_code ec;
        if (std::filesystem::exists(std::filesystem::path(localPath), ec))
        {
            this->m_Result.SetResult(DL_LOCALFILE_EXISTS, 0,
                                     L"[FileDownloader::DownloadFile] Fail : Local File Exists, FileName = %ls",
                                     this->m_pFileInfo->GetRemoteFilePath());
            return this->m_Result;
        }
    }

    // Create parent directories
    Path::CreateDirectorys(localPath, true);

    // Remove read-only attribute if needed
    std::error_code ec;
    auto perms = std::filesystem::status(std::filesystem::path(localPath), ec).permissions();
    if (!ec && (perms & std::filesystem::perms::owner_write) == std::filesystem::perms::none)
    {
        std::filesystem::permissions(std::filesystem::path(localPath), std::filesystem::perms::owner_write,
                                     std::filesystem::perm_options::add, ec);
    }

    // Convert local path to narrow string for ofstream
    char narrowLocalPath[MAX_PATH * 4] = {0};
    wcstombs(narrowLocalPath, localPath, sizeof(narrowLocalPath) - 1);

    // Open local file for writing
    this->m_LocalFile.open(narrowLocalPath, std::ios::binary | std::ios::trunc);
    if (!this->m_LocalFile.is_open())
    {
        this->m_Result.SetResult(DL_CREATE_LOCALFILE, 0,
                                 L"[FileDownloader::DownloadFile] Fail : CreateFile, FileName = %ls",
                                 this->m_pFileInfo->GetRemoteFilePath());
        return this->m_Result;
    }

    // Initialize curl
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        this->m_LocalFile.close();
        this->m_Result.SetResult(DL_CREATE_SESSION, 0, L"[FileDownloader::DownloadFile] Fail : curl_easy_init");
        return this->m_Result;
    }

    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set write callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FileDownloader::CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &this->m_LocalFile);

    // Set progress callback for break support
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, FileDownloader::CurlProgressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    // Timeouts (AC-STD-12: no hangs on network stalls)
    uint32_t connectTimeout = this->m_pServerInfo->GetConnectTimeout();
    if (connectTimeout > 0)
    {
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, static_cast<long>(connectTimeout));
    }
    else
    {
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    }
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // 5 minute overall timeout

    // Apply protocol-specific configuration
    this->m_pConnecter->ConfigureCurl(curl);

    // Notify start
    this->SendStartedDownloadFileEvent(0);

    // Perform download (optionally with timeout thread)
    CURLcode res;
    if (connectTimeout > 0)
    {
        // Use async for timeout support
        auto future = std::async(std::launch::async, [curl]() { return curl_easy_perform(curl); });

        auto status = future.wait_for(std::chrono::milliseconds(connectTimeout + 60000));
        if (status == std::future_status::timeout)
        {
            this->m_bBreak = true;
            res = future.get(); // Wait for curl to abort via progress callback
        }
        else
        {
            res = future.get();
        }
    }
    else
    {
        res = curl_easy_perform(curl);
    }

    // Get file size info
    curl_off_t downloadSize = 0;
    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &downloadSize);
    this->m_nFileLength = static_cast<uint64_t>(downloadSize);
    this->m_pFileInfo->SetFileLength(this->m_nFileLength);

    // Check result
    if (res == CURLE_OK)
    {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        if (httpCode >= 400)
        {
            this->m_Result.SetResult(DL_HTTP_STATUS_NOT_OK, static_cast<uint32_t>(httpCode),
                                     L"[FileDownloader::DownloadFile] Fail : HTTP status %ld", httpCode);
        }
        else
        {
            this->m_Result.SetSuccessResult();
        }
    }
    else if (res == CURLE_ABORTED_BY_CALLBACK)
    {
        this->m_Result.SetResult(WZ_USER_BREAK, 0, L"[FileDownloader] User Break");
    }
    else if (res == CURLE_OPERATION_TIMEDOUT)
    {
        this->m_Result.SetResult(DL_CONNECTION_TIMEOUT, 0,
                                 L"[FileDownloader::DownloadFile] Fail : TIMEOUT, FileName = %ls",
                                 this->m_pFileInfo->GetRemoteFilePath());
    }
    else
    {
        this->m_Result.SetResult(DL_CREATE_CONNECTION, static_cast<uint32_t>(res),
                                 L"[FileDownloader::DownloadFile] Fail : curl error %d", static_cast<int>(res));
    }

    curl_easy_cleanup(curl);
    this->m_LocalFile.close();

    // Remove file on failure
    if (!this->m_Result.IsSuccess())
    {
        std::filesystem::remove(std::filesystem::path(localPath), ec);
    }

    this->SendCompletedDownloadFileEvent(this->m_Result);

    return this->m_Result;
}

bool FileDownloader::CanBeContinue()
{
    if (this->m_bBreak)
        this->m_Result.SetResult(WZ_USER_BREAK, WZ_SUCCESS, L"[FileDownloader] User Break");
    return this->m_Result.IsSuccess();
}

void FileDownloader::Release()
{
    if (this->m_LocalFile.is_open())
        this->m_LocalFile.close();

    SAFE_DELETE(m_pConnecter);
}

IConnecter* FileDownloader::CreateConnecter()
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

void FileDownloader::SendStartedDownloadFileEvent(uint64_t nFileLength)
{
    if (this->m_pStateEvent != nullptr)
    {
        this->m_pStateEvent->OnStartedDownloadFile(this->m_pFileInfo->GetFileName(), nFileLength);
    }
}

void FileDownloader::SendCompletedDownloadFileEvent(WZResult wzResult)
{
    if (this->m_pStateEvent != nullptr)
    {
        this->m_pStateEvent->OnCompletedDownloadFile(this->m_pFileInfo->GetFileName(), wzResult);
    }
}

void FileDownloader::SendProgressDownloadFileEvent(uint64_t nTotalBytesRead)
{
    if (this->m_pStateEvent != nullptr)
    {
        this->m_pStateEvent->OnProgressDownloadFile(this->m_pFileInfo->GetFileName(), nTotalBytesRead);
    }
}

#endif
