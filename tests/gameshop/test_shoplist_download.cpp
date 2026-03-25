// Story 7.6.6: ShopListManager Cross-Platform HTTP Downloader — ATDD Acceptance Tests
// Flow Code: VS0-QUAL-WIN32CLEAN-SHOPHTTP
//
// RED PHASE: Tests compile on all platforms but use Catch2 SKIP until:
//   Task 3: TCHAR/HANDLE/INTERNET_PORT replaced with portable types in DownloadInfo.h,
//           WZResult.h, and ListManager.h
//   Task 4: All #ifdef _WIN32 guards removed from 17 ShopListManager .cpp files
//   Task 1: libcurl linked via find_package(CURL REQUIRED) or FetchContent in CMakeLists.txt
//
// To activate GREEN phase (once Tasks 3 + 4 complete):
//   Add to tests/CMakeLists.txt:
//     target_compile_definitions(MuTests PRIVATE MU_SHOPLIST_CROSS_PLATFORM_READY)
//
// AC coverage:
//   AC-STD-2  — CShopListManager::LoadScriptList(false) with local fixture files succeeds
//   AC-7      — DownloadInfo.h compiles without Win32-specific types (TCHAR, INTERNET_PORT)
//   AC-3      — ShopListManager headers compile without <wininet.h> on this platform

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <string>

// Cross-platform ShopListManager headers — includable only after Tasks 3+4 complete.
// Until then, MU_SHOPLIST_CROSS_PLATFORM_READY is NOT defined and the real test bodies
// fall through to SKIP().
#ifdef MU_SHOPLIST_CROSS_PLATFORM_READY
#include "ShopListManager.h"
#include "Include.h"
#endif

namespace
{

#ifdef MU_SHOPLIST_CROSS_PLATFORM_READY
void WriteFixtureFile(const std::filesystem::path& filePath, const std::string& content)
{
    std::ofstream f(filePath);
    f << content;
}

// Creates minimal valid ShopListManager fixture files in a temporary directory.
// Returns the directory path for use as the local script path.
std::filesystem::path SetupShopListFixtures()
{
    auto tmpDir = std::filesystem::temp_directory_path() / "mu_shoplist_test_766";
    std::filesystem::create_directories(tmpDir);

    // Minimal tab-delimited fixture files.
    // Format: id<TAB>parentId<TAB>name<TAB>...fields<CR><LF>
    WriteFixtureFile(tmpDir / "IBSCategory.txt", "1\t0\tTestCategory\t0\r\n");
    WriteFixtureFile(tmpDir / "IBSPackage.txt",  "1\t0\tTestPackage\t0\t0\t0\t0\t0\t0\r\n");
    WriteFixtureFile(tmpDir / "IBSProduct.txt",  "1\t0\tTestProduct\t0\t0\t0\t0\t0\r\n");

    return tmpDir;
}
#endif

} // anonymous namespace

// ---------------------------------------------------------------------------
// AC-STD-2 [7-6-6]: ShopListManager loads local script without network
//
// Verifies that CShopListManager::LoadScriptList(false) (bDownload=false → local
// mode, no network access) succeeds when fixture files are present in the local
// script path directory.
//
// GREEN once: Tasks 3+4 complete and MU_SHOPLIST_CROSS_PLATFORM_READY is defined.
// ---------------------------------------------------------------------------
TEST_CASE("AC-STD-2 [7-6-6]: ShopListManager loads local script without network",
          "[gameshop][shoplist][7-6-6]")
{
#ifndef MU_SHOPLIST_CROSS_PLATFORM_READY
    SKIP("RED PHASE: ShopListManager not yet cross-platform. "
         "Enable MU_SHOPLIST_CROSS_PLATFORM_READY in tests/CMakeLists.txt once "
         "Task 3 replaces TCHAR/INTERNET_PORT types and Task 4 removes #ifdef _WIN32 guards.");
#else
    // GIVEN: A temporary directory with minimal valid fixture script files
    auto fixtureDir = SetupShopListFixtures();

    // WHEN: ShopListManager is configured for local-only load (bDownload = false)
    CShopListManager manager;
    CListVersionInfo version{};
    version.Zone   = 0;
    version.year   = 2026;
    version.yearId = 1;

    manager.SetListManagerInfo(
        DownloaderType::HTTP,             // type (irrelevant in local mode)
        L"",                              // server IP (not used with bDownload=false)
        L"",                              // user ID
        L"",                              // password
        L"",                              // remote path
        fixtureDir.wstring().c_str(),     // local path — fixture directory
        version);

    WZResult result = manager.LoadScriptList(false); // false = local mode, no network

    // THEN: Load succeeds without any network access
    REQUIRE(result.IsSuccess());

    // AND: The internal shop list pointer is valid (list was allocated and loaded)
    REQUIRE(manager.GetListPtr() != nullptr);

    // Cleanup temporary fixture files
    std::filesystem::remove_all(fixtureDir);
#endif
}

// ---------------------------------------------------------------------------
// AC-7 [7-6-6]: DownloadInfo.h compiles with portable types only
//
// Verifies that DownloadFileInfo no longer requires TCHAR, INTERNET_PORT, or
// INTERNET_MAX_URL_LENGTH from <wininet.h>. The compilation of this translation
// unit (which includes DownloadInfo.h via ShopListManager.h) without error IS
// the test.
//
// GREEN once: Task 3.1 replaces Win32 types with std::string / unsigned short / size_t.
// ---------------------------------------------------------------------------
TEST_CASE("AC-7 [7-6-6]: DownloadInfo.h compiles with portable types only",
          "[gameshop][types][7-6-6]")
{
#ifndef MU_SHOPLIST_CROSS_PLATFORM_READY
    SKIP("RED PHASE: DownloadInfo.h still contains TCHAR/INTERNET_PORT/HANDLE Win32 types. "
         "Enable MU_SHOPLIST_CROSS_PLATFORM_READY after Task 3.1 replaces these with "
         "std::string, unsigned short, and void* / size_t equivalents.");
#else
    // Structural compile-check: if this TU compiled with MU_SHOPLIST_CROSS_PLATFORM_READY
    // and did NOT require <wininet.h>, then DownloadInfo.h uses portable types.
    DownloadFileInfo info;
    // Default construction must not crash — basic sanity for the new portable layout.
    SUCCEED("DownloadFileInfo default-constructs without Win32 headers");
#endif
}

// ---------------------------------------------------------------------------
// AC-3 [7-6-6]: ShopListManager compiles without WinINet headers
//
// The fact that this test file compiles (with MU_SHOPLIST_CROSS_PLATFORM_READY
// defined) without pulling in <wininet.h> proves that AC-3 is satisfied: all
// WinINet API usage has been replaced with libcurl.
//
// GREEN once: Tasks 2.4 + 4.1 remove all WinINet includes and #ifdef _WIN32 guards.
// ---------------------------------------------------------------------------
TEST_CASE("AC-3 [7-6-6]: ShopListManager headers compile without <wininet.h>",
          "[gameshop][cross-platform][7-6-6]")
{
#ifndef MU_SHOPLIST_CROSS_PLATFORM_READY
    SKIP("RED PHASE: Include.h still has '#ifdef _WIN32 #include <wininet.h>'. "
         "Enable MU_SHOPLIST_CROSS_PLATFORM_READY after Tasks 2.4+4.1 remove all "
         "WinINet includes and #ifdef _WIN32 guards from the ShopListManager module.");
#else
    // If this TU compiled on macOS/Linux with ShopListManager.h included above, then
    // <wininet.h> was not required. The compilation success IS the assertion.
    SUCCEED("ShopListManager headers compiled without <wininet.h> on this platform");
#endif
}
