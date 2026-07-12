// libcurl implementation of the shop file downloader (issue #462). Compiled
// only on non-Windows; on Windows the WinINet FileDownloader is used instead.

#include "stdafx.h"

#if !defined(_WIN32) && defined(KJH_ADD_INGAMESHOP_UI_SYSTEM)

#include "CurlFileDownloader.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "Data/Translation/MultiLanguage.h"
#include "GameShop/ShopListManager/interface/WZResult/ErrorCodeDefine.h"

namespace
{
// Mirrors the WinINet downloader's fixed 3s connect timeout.
constexpr long kConnectTimeoutMs = 3000;
// Abort a transfer that stalls below 1 byte/s for this long, so a dead server
// cannot hang the shop indefinitely (the non-Windows path runs inline, without
// the Windows watchdog thread).
constexpr long kLowSpeedLimitBytesPerSec = 1;
constexpr long kLowSpeedTimeoutSec = 30;

// libcurl needs one process-wide init before any easy handle is created.
// curl_easy_init would do it lazily, but not thread-safely; do it once here.
void EnsureCurlGlobalInit()
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
}

std::string WideToUtf8(const std::wstring& value)
{
    if (value.empty())
    {
        return std::string();
    }

    // ConvertToUtf8 treats its third argument as the destination capacity and
    // clamps to it; size for the worst-case 4-byte UTF-8 expansion plus a null.
    std::vector<char> buffer(value.size() * 4 + 1, '\0');
    CMultiLanguage::ConvertToUtf8(buffer.data(), value.c_str(), static_cast<int>(buffer.size()));
    return std::string(buffer.data());
}

std::wstring Utf8ToWide(const char* value)
{
    if (value == nullptr || *value == '\0')
    {
        return std::wstring();
    }

    std::vector<wchar_t> buffer(std::strlen(value) + 1, L'\0');
    CMultiLanguage::ConvertFromUtf8(buffer.data(), value, static_cast<int>(buffer.size()));
    return std::wstring(buffer.data());
}

// Progress callback: a non-zero return aborts the transfer. Used to honour the
// caller's break flag.
int XferInfoCallback(void* clientp, curl_off_t, curl_off_t, curl_off_t, curl_off_t)
{
    const volatile int* pBreak = static_cast<const volatile int*>(clientp);
    return (pBreak != nullptr && *pBreak != 0) ? 1 : 0;
}

size_t WriteToStream(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    auto* out = static_cast<std::ofstream*>(userdata);
    const size_t byteCount = size * nmemb;
    out->write(ptr, static_cast<std::streamsize>(byteCount));
    // A short return tells libcurl the write failed and aborts the transfer.
    return out->good() ? byteCount : 0;
}
} // namespace

WZResult CurlFileDownloader::DownloadFile(const std::wstring& url,
    const std::wstring& localPath,
    const std::wstring& username,
    const std::wstring& password,
    bool passiveFtp,
    const volatile int* pBreak)
{
    WZResult result;

    const std::filesystem::path targetPath(localPath);
    std::error_code fsError;
    if (targetPath.has_parent_path())
    {
        std::filesystem::create_directories(targetPath.parent_path(), fsError);
    }

    std::ofstream out(targetPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open())
    {
        result.SetResult(DL_CREATE_LOCALFILE, 0, L"[CurlFileDownloader] Fail : cannot open local file %ls", localPath.c_str());
        return result;
    }

    EnsureCurlGlobalInit();
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
    {
        result.SetResult(DL_CREATE_SESSION, 0, L"[CurlFileDownloader] Fail : curl_easy_init");
        return result;
    }

    const std::string urlUtf8 = WideToUtf8(url);
    const std::string userUtf8 = WideToUtf8(username);
    const std::string passwordUtf8 = WideToUtf8(password);

    curl_easy_setopt(curl, CURLOPT_URL, urlUtf8.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteToStream);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);      // HTTP >= 400 is a failure
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, kConnectTimeoutMs);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, kLowSpeedLimitBytesPerSec);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, kLowSpeedTimeoutSec);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, &XferInfoCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, const_cast<int*>(pBreak));
    curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, passiveFtp ? 1L : 0L);
    if (!passiveFtp)
    {
        curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");     // active mode
    }
    if (!userUtf8.empty())
    {
        curl_easy_setopt(curl, CURLOPT_USERNAME, userUtf8.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, passwordUtf8.c_str());
    }

    const CURLcode code = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    out.close();

    if (code == CURLE_OK)
    {
        result.SetSuccessResult();
        return result;
    }

    // Remove the partial file so a failed download is not mistaken for a good one.
    std::filesystem::remove(targetPath, fsError);

    if (code == CURLE_ABORTED_BY_CALLBACK)
    {
        result.SetResult(WZ_USER_BREAK, code, L"[CurlFileDownloader] User Break : %ls", url.c_str());
    }
    else
    {
        const std::wstring reason = Utf8ToWide(curl_easy_strerror(code));
        result.SetResult(DL_READ_REMOTEFILE, code, L"[CurlFileDownloader] Fail : %ls (%ls)", reason.c_str(), url.c_str());
    }

    return result;
}

#endif // !_WIN32 && KJH_ADD_INGAMESHOP_UI_SYSTEM
