// doctest unit tests for loading the in-game shop's script from the client folder: the shipped
// script loads without a download, a load never deletes it, and a byte order mark picks the
// script's encoding.

#include "App/stdafx.h"

#include <doctest.h>

#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Core/Utilities/Log/MuLogger.h"
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
        mu::log::Init(m_previous);
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

// The names of the entries in the working directory, as they are spelled on disk.
std::vector<std::string> WorkingDirectoryEntries()
{
    std::vector<std::string> names;
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path()))
    {
        names.push_back(entry.path().filename().string());
    }
    return names;
}

void WriteBytes(const std::filesystem::path& file, const std::string& bytes)
{
    std::ofstream(file, std::ios::binary) << bytes;
}

// UTF-16LE bytes of the text, with a byte order mark in front.
std::string Utf16LittleEndianWithMark(std::u16string_view text)
{
    std::string bytes = "\xFF\xFE";
    for (const char16_t unit : text)
    {
        bytes.push_back(static_cast<char>(unit & 0xFF));
        bytes.push_back(static_cast<char>(unit >> 8));
    }
    return bytes;
}

// The number and name of each category the shop list holds, in the script's order.
std::vector<std::pair<int, std::wstring>> LoadedCategories(CShopListManager& manager)
{
    std::vector<std::pair<int, std::wstring>> categories;
    CShopCategoryList* list = manager.GetListPtr()->GetCategoryListPtr();
    CShopCategory category;
    for (int index = 0; list->GetValueByIndex(index, category); ++index)
    {
        categories.emplace_back(category.ProductDisplaySeq, category.CategroyName);
    }
    return categories;
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
    // The folder isn't created a second time in the requested spelling. Listed instead of asking
    // for "Data", which a case-insensitive file system (Windows, macOS) reports for "data" too.
    CHECK(WorkingDirectoryEntries() == std::vector<std::string>{"data"});
}

TEST_CASE("the shop reads a script in each encoding its byte order mark names [ui][ingameshop]")
{
    const ScopedWorkingDirectory directory("mu_shop_script_encodings");
    const std::filesystem::path folder = "Data/InGameShopScript/512.2012.084";
    WriteFiles(folder, {"IBSCategory.txt", "IBSPackage.txt", "IBSProduct.txt"});
    const std::vector<std::pair<int, std::wstring>> expected{{10, L"First"}, {20, L"Second"}};

    SUBCASE("no byte order mark")
    {
        WriteBytes(folder / "IBSCategory.txt", "10@First@0@1@0@1@1\r\n20@Second@0@1@0@2@1\r\n");
        CShopListManager manager;
        REQUIRE(LoadShippedScript(manager).IsSuccess());
        CHECK(LoadedCategories(manager) == expected);
    }
    SUBCASE("UTF-8: the mark is no part of the first row's number")
    {
        WriteBytes(folder / "IBSCategory.txt", "\xEF\xBB\xBF" "10@First@0@1@0@1@1\r\n20@Second@0@1@0@2@1\r\n");
        CShopListManager manager;
        REQUIRE(LoadShippedScript(manager).IsSuccess());
        CHECK(LoadedCategories(manager) == expected);
    }
    SUBCASE("UTF-16: every row, also after one with a character that holds a 0x0A byte")
    {
        // U+010A is the bytes 0A 01, which a text-mode line reader took for a line break.
        WriteBytes(folder / "IBSCategory.txt",
                   Utf16LittleEndianWithMark(u"10@First\x010A@0@1@0@1@1\r\n20@Second@0@1@0@2@1\r\n"));
        CShopListManager manager;
        REQUIRE(LoadShippedScript(manager).IsSuccess());
        CHECK(LoadedCategories(manager) ==
              std::vector<std::pair<int, std::wstring>>{{10, L"First\x010A"}, {20, L"Second"}});
    }
}
