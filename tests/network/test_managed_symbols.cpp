#include "doctest.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Dotnet/ManagedSymbols.h"

#include <spdlog/sinks/ostream_sink.h>

#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using DotNetBridge::ManagedExports;
using DotNetBridge::ManagedSymbolsState;
using DotNetBridge::ManagedSymbolTable;

namespace
{
// What the "dotnet" logger writes while it lives.
class DotnetLogCapture
{
public:
    DotnetLogCapture()
        : m_logger(mu::log::Get("dotnet")), m_sink(std::make_shared<spdlog::sinks::ostream_sink_st>(m_text))
    {
        m_logger->sinks().push_back(m_sink);
    }
    ~DotnetLogCapture()
    {
        std::erase(m_logger->sinks(), m_sink);
    }
    DotnetLogCapture(const DotnetLogCapture&) = delete;
    DotnetLogCapture& operator=(const DotnetLogCapture&) = delete;

    std::string Text() const
    {
        return m_text.str();
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
    std::ostringstream m_text;
    std::shared_ptr<spdlog::sinks::ostream_sink_st> m_sink;
};

// A fake source: it has every export except the one named here.
const char* g_lacking = nullptr;
int g_export = 0;

void* FindAllButOne(const char* name)
{
    return g_lacking != nullptr && std::strcmp(name, g_lacking) == 0 ? nullptr : &g_export;
}
} // namespace

TEST_CASE("Every listed export resolves from a source that has them all [network][dotnet]")
{
    g_lacking = nullptr;
    ManagedSymbolTable table;

    CHECK(table.Resolve({"the fake exports", &FindAllButOne}, ManagedExports()) == ManagedSymbolsState::Resolved);
    CHECK(table.Find("SendPublicChatMessage") == &g_export);
    CHECK(table.Find("ConnectionManager_BeginConnect") == &g_export);
    CHECK(table.Find("ConnectionManager_SendLogin") == &g_export);
}

TEST_CASE("A missing export fails the resolve, and the log names it [network][dotnet]")
{
    g_lacking = "SendPublicChatMessage";
    const DotnetLogCapture log;
    ManagedSymbolTable table;

    CHECK(table.Resolve({"the fake exports", &FindAllButOne}, ManagedExports()) == ManagedSymbolsState::ExportsMissing);
    CHECK(table.MissingExports() == std::vector<std::string>{"SendPublicChatMessage"});
    CHECK(log.Text().find("SendPublicChatMessage") != std::string::npos);
    // The library doesn't match this client, so none of it is used.
    CHECK(table.Find("SendKeepAlive") == nullptr);
}

TEST_CASE("An optional export may be missing [network][dotnet]")
{
    g_lacking = "ConnectionManager_GetLibraryVersion";
    ManagedSymbolTable table;

    CHECK(table.Resolve({"the fake exports", &FindAllButOne}, ManagedExports()) == ManagedSymbolsState::Resolved);
    CHECK(table.Find("ConnectionManager_GetLibraryVersion") == nullptr);
    CHECK(table.Find("SendKeepAlive") == &g_export);
}

TEST_CASE("A resolved table keeps its exports when resolved again [network][dotnet]")
{
    g_lacking = nullptr;
    ManagedSymbolTable table;
    REQUIRE(table.Resolve({"the fake exports", &FindAllButOne}, ManagedExports()) == ManagedSymbolsState::Resolved);

    CHECK(table.Resolve({"another source", nullptr}, ManagedExports()) == ManagedSymbolsState::Resolved);
    CHECK(table.Find("SendKeepAlive") == &g_export);
    CHECK(table.Description() == "the fake exports");
}

TEST_CASE("A missing source leaves every export unresolved [network][dotnet]")
{
    ManagedSymbolTable table;

    CHECK(table.Resolve({"no library", nullptr}, ManagedExports()) == ManagedSymbolsState::SourceMissing);
    CHECK(table.Find("SendKeepAlive") == nullptr);
    CHECK(table.Description() == "no library");
}

TEST_CASE("A lookup before the exports are resolved is logged [network][dotnet]")
{
    // Nothing in this test binary resolves the process's table.
    REQUIRE(DotNetBridge::ProcessManagedSymbols().State() == ManagedSymbolsState::NotResolved);
    const DotnetLogCapture log;

    CHECK(DotNetBridge::LookUpManagedSymbol("SendKeepAlive") == nullptr);
    CHECK(log.Text().find("SendKeepAlive") != std::string::npos);
}
