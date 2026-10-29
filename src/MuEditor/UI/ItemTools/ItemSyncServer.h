#pragma once

#ifdef _EDITOR

#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

// Update from server of the Item Sync window (phase 7 step 4): downloads OpenMU's item exchange
// file from its admin panel, GET <admin panel URL>/api/items/exchange with the API key in the
// X-Api-Key header. The key is typed into the window, or comes from the environment variable
// MU_OPENMU_API_KEY; Remember keeps it in the system's key store (SecretStore), never in a file
// or a log. The download runs on a thread of its own and can be cancelled.
namespace MuEditor::Items
{
// The environment variable with the API key of the admin panel (the Viewer role is enough).
constexpr const char* OpenMUApiKeyVariable = "MU_OPENMU_API_KEY";
// The path of the exchange file below the admin panel's address.
constexpr const char* OpenMUExchangePath = "/api/items/exchange";

// Why an update from the server has no file.
enum class ServerUpdateProblem : uint8_t
{
    None,
    // No address of the admin panel.
    NoUrl,
    // Not https, and not http to this computer (localhost, 127.0.0.1, [::1]).
    InsecureUrl,
    NoKey,
    // The server could not be reached (or did not answer in time).
    NoConnection,
    // 401: the admin panel does not know the key.
    KeyRefused,
    // 403: the key lacks the Viewer role, or the admin panel has no user yet.
    NotAllowed,
    // 404: the admin panel has no exchange file (an OpenMU without it).
    NoEndpoint,
    // 429: too many requests.
    TooManyRequests,
    // Any other answer than 200.
    OtherStatus,
    Cancelled,
};

// The address of the exchange file for the admin panel's address (`adminPanelUrl` may end with
// a "/" and have a path, e.g. behind a reverse proxy), or nothing with `problem` set: the address
// must be https, or http to this computer.
std::optional<std::string> GetExchangeUrl(std::string_view adminPanelUrl, ServerUpdateProblem& problem);

// The problem an answer's HTTP status gives; None for 200.
ServerUpdateProblem ProblemOfStatus(long status);

// The API key from MU_OPENMU_API_KEY; empty when it is not set.
std::string ReadOpenMUApiKey();
// The name the key store keeps the API key of the admin panel at `adminPanelUrl` under: one key
// per address ("OpenMU API key https://admin.example.org").
std::string GetApiKeySecretName(std::string_view adminPanelUrl);

struct ServerDownload
{
    ServerUpdateProblem problem = ServerUpdateProblem::None;
    // The HTTP status of the answer; 0 without one.
    long status = 0;
    // What libcurl says when the server could not be reached.
    std::string detail;
    // The file, when there is no problem.
    std::string body;
};

// One download at a time, on a thread of its own.
class CItemSyncDownload
{
public:
    CItemSyncDownload() = default;
    CItemSyncDownload(const CItemSyncDownload&) = delete;
    CItemSyncDownload& operator=(const CItemSyncDownload&) = delete;
    // Cancels a running download and waits for its thread.
    ~CItemSyncDownload();

    // Starts the download of `url` with `apiKey`; false while one runs.
    bool Start(std::string url, std::string apiKey);
    // Asks a running download to stop; it ends as Cancelled.
    void Cancel();
    bool IsRunning() const
    {
        return m_running;
    }
    // The finished download, once; nothing while it runs or after it was taken.
    std::optional<ServerDownload> TakeResult();

private:
    void Join();

    std::thread m_thread;
    std::atomic_bool m_running = false;
    std::atomic_bool m_cancel = false;
    std::mutex m_mutex;
    std::optional<ServerDownload> m_result;
};
} // namespace MuEditor::Items

#endif // _EDITOR
