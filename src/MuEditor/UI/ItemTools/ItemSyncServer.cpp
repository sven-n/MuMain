#include "stdafx.h"

#ifdef _EDITOR

#include "ItemSyncServer.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <utility>

#include <curl/curl.h>

namespace MuEditor::Items
{
namespace
{
// How long the server may take to accept the connection, and to send the whole file.
constexpr long ConnectTimeoutMs = 10'000;
constexpr long TransferTimeoutMs = 120'000;

// The hosts plain http may reach: this computer only.
constexpr std::string_view LocalHosts[] = {"localhost", "127.0.0.1", "[::1]"};

std::string ToLower(std::string_view text)
{
    std::string lower(text);
    std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

std::string_view Trim(std::string_view text)
{
    const auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    while (!text.empty() && isSpace(static_cast<unsigned char>(text.front())))
    {
        text.remove_prefix(1);
    }
    while (!text.empty() && isSpace(static_cast<unsigned char>(text.back())))
    {
        text.remove_suffix(1);
    }
    return text;
}

// The host of `rest`, the part of an address after "scheme://": up to the port, path, query or
// fragment; an IPv6 address keeps its brackets.
std::string_view HostOf(std::string_view rest)
{
    if (rest.starts_with('['))
    {
        const size_t end = rest.find(']');
        return end == std::string_view::npos ? rest : rest.substr(0, end + 1);
    }
    return rest.substr(0, rest.find_first_of(":/?#"));
}

size_t AppendBody(char* data, size_t size, size_t count, void* userdata)
{
    static_cast<std::string*>(userdata)->append(data, size * count);
    return size * count;
}

int OnProgress(void* userdata, curl_off_t, curl_off_t, curl_off_t, curl_off_t)
{
    // A non-zero answer aborts the transfer (CURLE_ABORTED_BY_CALLBACK).
    return static_cast<const std::atomic_bool*>(userdata)->load() ? 1 : 0;
}

// libcurl needs one process-wide init before any easy handle; it runs on the window's thread,
// before the download thread starts.
void EnsureCurlGlobalInit()
{
    static std::once_flag once;
    std::call_once(once, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
}

ServerDownload Download(const std::string& url, const std::string& apiKey, const std::atomic_bool& cancel)
{
    ServerDownload result;
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
    {
        result.problem = ServerUpdateProblem::NoConnection;
        result.detail = "libcurl could not start";
        return result;
    }
    const std::string keyHeader = std::string("X-Api-Key: ") + apiKey;
    curl_slist* headers = curl_slist_append(nullptr, keyHeader.c_str());
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &AppendBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.body);
    // A redirect could take the key to another host.
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, ConnectTimeoutMs);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, TransferTimeoutMs);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, &OnProgress);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, const_cast<std::atomic_bool*>(&cancel));

    const CURLcode code = curl_easy_perform(curl);
    if (code == CURLE_ABORTED_BY_CALLBACK)
    {
        result.problem = ServerUpdateProblem::Cancelled;
    }
    else if (code != CURLE_OK)
    {
        result.problem = ServerUpdateProblem::NoConnection;
        result.detail = curl_easy_strerror(code);
    }
    else
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);
        result.problem = ProblemOfStatus(result.status);
    }
    if (result.problem != ServerUpdateProblem::None)
    {
        result.body.clear();
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return result;
}
} // namespace

std::optional<std::string> GetExchangeUrl(std::string_view adminPanelUrl, ServerUpdateProblem& problem)
{
    const std::string_view url = Trim(adminPanelUrl);
    if (url.empty())
    {
        problem = ServerUpdateProblem::NoUrl;
        return std::nullopt;
    }
    const std::string lower = ToLower(url);
    constexpr std::string_view Https = "https://";
    constexpr std::string_view Http = "http://";
    bool allowed = lower.starts_with(Https) && lower.size() > Https.size();
    if (!allowed && lower.starts_with(Http))
    {
        const std::string_view host = HostOf(std::string_view(lower).substr(Http.size()));
        allowed = std::ranges::find(LocalHosts, host) != std::end(LocalHosts);
    }
    if (!allowed)
    {
        problem = ServerUpdateProblem::InsecureUrl;
        return std::nullopt;
    }
    std::string exchange(url);
    while (exchange.ends_with('/'))
    {
        exchange.pop_back();
    }
    problem = ServerUpdateProblem::None;
    return exchange + OpenMUExchangePath;
}

ServerUpdateProblem ProblemOfStatus(long status)
{
    switch (status)
    {
    case 200:
        return ServerUpdateProblem::None;
    case 401:
        return ServerUpdateProblem::KeyRefused;
    case 403:
        return ServerUpdateProblem::NotAllowed;
    case 404:
        return ServerUpdateProblem::NoEndpoint;
    case 429:
        return ServerUpdateProblem::TooManyRequests;
    default:
        return ServerUpdateProblem::OtherStatus;
    }
}

std::string ReadOpenMUApiKey()
{
    const char* key = std::getenv(OpenMUApiKeyVariable);
    return key != nullptr ? std::string(Trim(key)) : std::string();
}

std::string GetApiKeySecretName(std::string_view adminPanelUrl)
{
    std::string_view url = Trim(adminPanelUrl);
    while (!url.empty() && url.back() == '/')
    {
        url.remove_suffix(1);
    }
    return "OpenMU API key " + std::string(url);
}

CItemSyncDownload::~CItemSyncDownload()
{
    Cancel();
    Join();
}

bool CItemSyncDownload::Start(std::string url, std::string apiKey)
{
    if (m_running)
    {
        return false;
    }
    Join();
    EnsureCurlGlobalInit();
    {
        std::lock_guard lock(m_mutex);
        m_result.reset();
    }
    m_cancel = false;
    m_running = true;
    m_thread = std::thread(
        [this, url = std::move(url), apiKey = std::move(apiKey)]
        {
            ServerDownload result = Download(url, apiKey, m_cancel);
            {
                std::lock_guard lock(m_mutex);
                m_result = std::move(result);
            }
            m_running = false;
        });
    return true;
}

void CItemSyncDownload::Cancel()
{
    m_cancel = true;
}

std::optional<ServerDownload> CItemSyncDownload::TakeResult()
{
    std::optional<ServerDownload> result;
    {
        std::lock_guard lock(m_mutex);
        result = std::exchange(m_result, std::nullopt);
    }
    if (result)
    {
        Join();
    }
    return result;
}

void CItemSyncDownload::Join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
