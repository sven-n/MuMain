#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "UI/ItemTools/ItemSyncServer.h"

#include <chrono>
#include <optional>
#include <string>
#include <thread>

using namespace MuEditor::Items;

// Update from server of the Item Sync window: the address it downloads from, and what an answer
// of the admin panel means. The download itself needs a server and is tried by hand.
namespace
{
std::optional<std::string> UrlFor(const std::string& adminPanelUrl, ServerUpdateProblem& problem)
{
    problem = ServerUpdateProblem::OtherStatus;
    return GetExchangeUrl(adminPanelUrl, problem);
}
} // namespace

TEST_CASE("Update from server downloads from the admin panel's address [data][items][editor]")
{
    ServerUpdateProblem problem;

    CHECK(UrlFor("https://admin.example.org", problem) == "https://admin.example.org/api/items/exchange");
    CHECK(problem == ServerUpdateProblem::None);
    // A trailing slash, a path below a reverse proxy and spaces around it.
    CHECK(UrlFor(" https://example.org/openmu/ ", problem) == "https://example.org/openmu/api/items/exchange");
    CHECK(UrlFor("HTTPS://Example.org", problem) == "HTTPS://Example.org/api/items/exchange");
    // Plain http only to this computer.
    CHECK(UrlFor("http://localhost:8080", problem) == "http://localhost:8080/api/items/exchange");
    CHECK(UrlFor("http://127.0.0.1", problem) == "http://127.0.0.1/api/items/exchange");
    CHECK(UrlFor("http://[::1]:5000/", problem) == "http://[::1]:5000/api/items/exchange");
    CHECK(UrlFor("http://LocalHost", problem) == "http://LocalHost/api/items/exchange");
}

TEST_CASE("Update from server refuses an address without https [data][items][editor]")
{
    ServerUpdateProblem problem;

    CHECK_FALSE(UrlFor("", problem));
    CHECK(problem == ServerUpdateProblem::NoUrl);
    CHECK_FALSE(UrlFor("   ", problem));
    CHECK(problem == ServerUpdateProblem::NoUrl);
    for (const char* url : {"http://admin.example.org", "http://localhost.example.org", "http://127.0.0.1.nip.io",
                            "http://192.168.1.10", "ftp://localhost", "localhost:8080", "https://"})
    {
        CAPTURE(url);
        CHECK_FALSE(UrlFor(url, problem));
        CHECK(problem == ServerUpdateProblem::InsecureUrl);
    }
}

TEST_CASE("Update from server tells the admin panel's answers apart [data][items][editor]")
{
    CHECK(ProblemOfStatus(200) == ServerUpdateProblem::None);
    CHECK(ProblemOfStatus(401) == ServerUpdateProblem::KeyRefused);
    CHECK(ProblemOfStatus(403) == ServerUpdateProblem::NotAllowed);
    CHECK(ProblemOfStatus(404) == ServerUpdateProblem::NoEndpoint);
    CHECK(ProblemOfStatus(429) == ServerUpdateProblem::TooManyRequests);
    CHECK(ProblemOfStatus(500) == ServerUpdateProblem::OtherStatus);
    CHECK(ProblemOfStatus(302) == ServerUpdateProblem::OtherStatus);
}

TEST_CASE("Update from server reports a server it cannot reach [data][items][editor]")
{
    // Port 9 of this computer (discard) has nothing listening on a test machine.
    CItemSyncDownload download;
    REQUIRE(download.Start("http://127.0.0.1:9/api/items/exchange", "omu_test-key"));
    std::optional<ServerDownload> result;
    for (int attempt = 0; attempt < 300 && !result; ++attempt)
    {
        result = download.TakeResult();
        if (!result)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    REQUIRE(result);
    CHECK(result->problem == ServerUpdateProblem::NoConnection);
    CHECK_FALSE(result->detail.empty());
    CHECK(result->body.empty());
}

#endif // _EDITOR
