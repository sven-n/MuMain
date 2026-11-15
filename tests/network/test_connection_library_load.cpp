#include "doctest.h"

#include "Core/Platform/PlatformLibrary.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <spdlog/sinks/ostream_sink.h>

#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Dotnet/Connection.h"
#include "Dotnet/SymbolSources/ManagedLibrarySource.h"

#ifndef _WIN32
#include "Core/Platform/SecureCrt.h"
#include "Core/Platform/WinApiShims.h"
#endif

TEST_CASE("Connection library loader handles missing libraries [network][dotnet]")
{
    const auto handle = mu::platform::Load("NonExistent.Client.Library.xyz");
    CHECK(handle == nullptr);
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_BeginConnect") == nullptr);
    mu::platform::Unload(handle);
}

namespace
{
// A library every test machine has, with one symbol it exports.
#ifdef _WIN32
constexpr const char* kSystemLibrary = "kernel32.dll";
constexpr const char* kSystemSymbol = "GetTickCount";
#elif defined(__APPLE__)
constexpr const char* kSystemLibrary = "/usr/lib/libSystem.B.dylib";
constexpr const char* kSystemSymbol = "strlen";
#else
constexpr const char* kSystemLibrary = "libc.so.6";
constexpr const char* kSystemSymbol = "strlen";
#endif
constexpr const char* kMissingSymbol = "MuMain_NoSuchExport";

// What the "platform" logger writes while it lives.
class PlatformLogCapture
{
public:
    PlatformLogCapture()
        : m_logger(mu::log::Get("platform")), m_sink(std::make_shared<spdlog::sinks::ostream_sink_st>(m_text))
    {
        m_logger->sinks().push_back(m_sink);
    }
    ~PlatformLogCapture()
    {
        std::erase(m_logger->sinks(), m_sink);
    }
    PlatformLogCapture(const PlatformLogCapture&) = delete;
    PlatformLogCapture& operator=(const PlatformLogCapture&) = delete;

    std::string Text() const
    {
        return m_text.str();
    }

private:
    std::ostringstream m_text;
    std::shared_ptr<spdlog::logger> m_logger;
    std::shared_ptr<spdlog::sinks::ostream_sink_st> m_sink;
};
} // namespace

TEST_CASE("A symbol the library lacks is found as null without an error line [network][dotnet]")
{
    const auto handle = mu::platform::Load(kSystemLibrary);
    REQUIRE(handle != nullptr);

    SUBCASE("FindSymbol finds what the library has")
    {
        CHECK(mu::platform::FindSymbol(handle, kSystemSymbol) != nullptr);
    }
    SUBCASE("FindSymbol logs nothing for a missing symbol")
    {
        const PlatformLogCapture log;
        CHECK(mu::platform::FindSymbol(handle, kMissingSymbol) == nullptr);
        CHECK(mu::platform::FindSymbol(nullptr, kMissingSymbol) == nullptr);
        CHECK(log.Text().empty());
    }
    SUBCASE("GetSymbol still logs a missing symbol")
    {
        const PlatformLogCapture log;
        CHECK(mu::platform::GetSymbol(handle, kMissingSymbol) == nullptr);
        CHECK(log.Text().find(kMissingSymbol) != std::string::npos);
    }
    mu::platform::Unload(handle);
}

TEST_CASE("The network library loads at startup, once, not during static initialisation [network][dotnet]")
{
    // Nothing in this test binary loads it before this line.
    CHECK(munique_client_library_handle == nullptr);

    std::string firstLog;
    {
        const PlatformLogCapture log;
        LoadManagedLibrary();
        firstLog = log.Text();
    }
    const auto handle = munique_client_library_handle;
    // Loaded, or its failure logged: the test binary may not have the library beside it.
    CHECK((handle != nullptr || !firstLog.empty()));

    const PlatformLogCapture log;
    LoadManagedLibrary();
    CHECK(munique_client_library_handle == handle);
    CHECK(log.Text().empty());
}

TEST_CASE("The library is found by its configured name, or beside the executable [network][dotnet]")
{
    if constexpr (MU_MANAGED_LIBRARY_BY_NAME)
    {
        CHECK(ManagedLibraryPath() == MU_MANAGED_LIBRARY_FILE);
    }
    else
    {
        constexpr DWORD executablePathCapacity = 4096;
        wchar_t executablePath[executablePathCapacity] = {};
        REQUIRE(GetModuleFileNameW(nullptr, executablePath, executablePathCapacity) != 0);

        const auto expectedPath =
            std::filesystem::path(mu_narrow_path(executablePath)).parent_path() / MU_MANAGED_LIBRARY_FILE;
        CHECK(std::filesystem::path(ManagedLibraryPath()) == expectedPath);
    }
}
