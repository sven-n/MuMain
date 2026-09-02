#include "doctest.h"

#include "Core/Platform/PlatformLibrary.h"

#ifndef _WIN32
#include <filesystem>
#include <string>

#include "Core/Platform/SecureCrt.h"
#include "Core/Platform/WinApiShims.h"
#include "Dotnet/Connection.h"
#endif

TEST_CASE("Connection library loader handles missing libraries [network][dotnet]")
{
    const auto handle = mu::platform::Load("NonExistent.Client.Library.xyz");
    CHECK(handle == nullptr);
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_Connect") == nullptr);
    mu::platform::Unload(handle);
}

#ifndef _WIN32
TEST_CASE("Connection library path follows the executable [network][dotnet]")
{
    constexpr DWORD executablePathCapacity = 4096;
    wchar_t executablePath[executablePathCapacity] = {};
    REQUIRE(GetModuleFileNameW(nullptr, executablePath, executablePathCapacity) != 0);

    const auto libraryName = "MUnique.Client.Library" + std::string(MU_DOTNET_LIB_EXT);
    const auto expectedPath = std::filesystem::path(mu_narrow_path(executablePath)).parent_path() / libraryName;
    CHECK(std::filesystem::path(ManagedLibraryPath()) == expectedPath);
}
#endif
