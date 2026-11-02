// doctest unit tests for loading the in-game shop's script from the client folder: the shipped
// script loads without a download, and a load never deletes it.

#include "App/stdafx.h"

#include <doctest.h>

#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <string>

#include "GameShop/ShopListManager/ShopListManager.h"

namespace
{
constexpr unsigned short kSaleZone = 512;
constexpr unsigned short kYear = 2012;
constexpr unsigned short kYearId = 84;

// Runs a test in its own empty working directory, like the client in its folder.
class ScopedWorkingDirectory
{
public:
    explicit ScopedWorkingDirectory(const char* name)
        : m_previous(std::filesystem::current_path()), m_directory(std::filesystem::temp_directory_path() / name)
    {
        std::filesystem::remove_all(m_directory);
        std::filesystem::create_directories(m_directory);
        std::filesystem::current_path(m_directory);
    }

    ~ScopedWorkingDirectory()
    {
        std::filesystem::current_path(m_previous);
        std::error_code error;
        std::filesystem::remove_all(m_directory, error);
    }

    ScopedWorkingDirectory(const ScopedWorkingDirectory&) = delete;
    ScopedWorkingDirectory& operator=(const ScopedWorkingDirectory&) = delete;

private:
    std::filesystem::path m_previous;
    std::filesystem::path m_directory;
};

void WriteFiles(const std::filesystem::path& folder, std::initializer_list<const char*> names)
{
    std::filesystem::create_directories(folder);
    for (const char* name : names)
    {
        std::ofstream(folder / name) << "1@Test@0@0@1@1@1\n";
    }
}

WZResult LoadShippedScript(CShopListManager& manager)
{
    CListVersionInfo version{};
    version.Zone = kSaleZone;
    version.year = kYear;
    version.yearId = kYearId;
    // The server address doesn't matter: a found script is never downloaded.
    manager.SetListManagerInfo(HTTP, L"127.0.0.1", L"", L"", L"/remote", L"Data/InGameShopScript", version, 0);
    return manager.LoadScriptList(false);
}
} // namespace

TEST_CASE("the shop loads the shipped script and keeps it [ui][ingameshop]")
{
    const ScopedWorkingDirectory directory("mu_shop_script_shipped");
    const std::filesystem::path folder = "Data/InGameShopScript/512.2012.084";
    WriteFiles(folder, {"IBSCategory.txt", "IBSPackage.txt", "IBSProduct.txt"});

    CShopListManager manager;
    CHECK(LoadShippedScript(manager).IsSuccess());

    CHECK(std::filesystem::exists(folder / "IBSCategory.txt"));
    CHECK(std::filesystem::exists(folder / "IBSPackage.txt"));
    CHECK(std::filesystem::exists(folder / "IBSProduct.txt"));
}

TEST_CASE("the shop loads a script whose folder and files are spelled differently [ui][ingameshop]")
{
    // On a case-sensitive file system the check and the delete found these files, but the open
    // didn't, so every load deleted them.
    const ScopedWorkingDirectory directory("mu_shop_script_spelling");
    const std::filesystem::path folder = "data/ingameshopscript/512.2012.084";
    WriteFiles(folder, {"ibscategory.txt", "ibspackage.txt", "ibsproduct.txt"});

    CShopListManager manager;
    CHECK(LoadShippedScript(manager).IsSuccess());

    CHECK(std::filesystem::exists(folder / "ibscategory.txt"));
    CHECK(std::filesystem::exists(folder / "ibspackage.txt"));
    CHECK(std::filesystem::exists(folder / "ibsproduct.txt"));
#ifndef _WIN32
    // The folder isn't created a second time in the requested spelling.
    CHECK_FALSE(std::filesystem::exists("Data"));
#endif
}
