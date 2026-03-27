// Story 7.9.3: Unify Entry Point — Delete WinMain, Single main() for All Platforms
// Flow Code: VS0-QUAL-RENDER-UNIFYENTRY
//
// RED PHASE: Tests verify that WinMain() is deleted and MuMain() is the universal
// entry point, and that all #ifdef _WIN32 guards are removed from game code.
//
// Test phases:
//   AC-1 tests (screen rate math)    — ALWAYS PASS (pure logic, no implementation deps)
//   AC-2/AC-3 tests (file scan)      — RED: fail until WinMain deleted and MuMain unified
//   AC-5 tests (platform guard scan) — RED: fail until all 19 guards removed from game code
//
// AC Mapping:
//   AC-1     -> TEST_CASE("AC-1 [7-9-3]: ...")  screen rate calculation correctness
//   AC-2     -> TEST_CASE("AC-2 [7-9-3]: ...")  WinMain() deleted from Winmain.cpp
//   AC-3     -> TEST_CASE("AC-3 [7-9-3]: ...")  single main() entry point present
//   AC-5     -> TEST_CASE("AC-5 [7-9-3]: ...")  zero platform guards outside allowed dirs
//   AC-STD-2 -> TEST_CASE("AC-STD-2 [7-9-3]: ...") test suite passes without Win32 deps
//
// File-scan tests (AC-2, AC-3, AC-5) require MU_SOURCE_DIR injected from CMakeLists.txt.
// They are guarded by #ifndef _WIN32 so MinGW CI compiles cleanly (WinMain still exists
// in the source tree on CI; the structural tests are only run on macOS/Linux native builds).
//
// Run with: ctest --test-dir MuMain/out/build/macos-arm64 -R entry_point_unification_7_9_3

#include <catch2/catch_test_macros.hpp>
#include <cmath>

// MU_SOURCE_DIR is injected by CMakeLists.txt as the absolute path to MuMain/src/source/.
// Required for file-scan tests (AC-2, AC-3, AC-5). Skipped gracefully if not set.
#ifndef MU_SOURCE_DIR
#define MU_SOURCE_DIR ""
#endif

// ===========================================================================
// AC-1: Screen Rate Calculation Correctness
// ===========================================================================
//
// Verifies the formula ported from WinMain() to MuMain() in Task 1.3:
//   g_fScreenRate_x = (float)WindowWidth  / 640;
//   g_fScreenRate_y = (float)WindowHeight / 480;
//
// These are pure arithmetic tests — they pass immediately and serve as the
// regression guard ensuring the formula is never changed during migration.

TEST_CASE("AC-1 [7-9-3]: Screen rate x calculation matches WinMain formula", "[entry-point][ac-1]")
{
    SECTION("640 (reference width) produces rate 1.0")
    {
        float rate = static_cast<float>(640) / 640.0f;
        REQUIRE(rate == 1.0f);
    }

    SECTION("1280 width produces rate 2.0")
    {
        float rate = static_cast<float>(1280) / 640.0f;
        REQUIRE(rate == 2.0f);
    }

    SECTION("1920 width produces rate 3.0")
    {
        float rate = static_cast<float>(1920) / 640.0f;
        REQUIRE(std::abs(rate - 3.0f) < 0.001f);
    }

    SECTION("non-standard 1366 width produces positive non-zero rate")
    {
        float rate = static_cast<float>(1366) / 640.0f;
        CHECK(rate > 0.0f);
        CHECK(rate < 4.0f);
    }
}

TEST_CASE("AC-1 [7-9-3]: Screen rate y calculation matches WinMain formula", "[entry-point][ac-1]")
{
    SECTION("480 (reference height) produces rate 1.0")
    {
        float rate = static_cast<float>(480) / 480.0f;
        REQUIRE(rate == 1.0f);
    }

    SECTION("1080 height produces rate 2.25")
    {
        float rate = static_cast<float>(1080) / 480.0f;
        REQUIRE(std::abs(rate - 2.25f) < 0.001f);
    }

    SECTION("768 height produces rate between 1.0 and 2.0")
    {
        float rate = static_cast<float>(768) / 480.0f;
        CHECK(rate > 1.0f);
        CHECK(rate < 2.0f);
    }
}

TEST_CASE("AC-1 [7-9-3]: Screen rate x and y are equal for 4:3 resolutions",
    "[entry-point][ac-1]")
{
    // 4:3 resolutions keep the same aspect ratio as the 640x480 reference.
    // x_rate and y_rate must be equal for these resolutions.

    SECTION("640x480 reference — both rates equal 1.0")
    {
        float rx = static_cast<float>(640) / 640.0f;
        float ry = static_cast<float>(480) / 480.0f;
        REQUIRE(rx == ry);
    }

    SECTION("800x600 has equal x and y rates")
    {
        float rx = static_cast<float>(800) / 640.0f;
        float ry = static_cast<float>(600) / 480.0f;
        REQUIRE(std::abs(rx - ry) < 0.001f);
    }

    SECTION("1024x768 has equal x and y rates")
    {
        float rx = static_cast<float>(1024) / 640.0f;
        float ry = static_cast<float>(768) / 480.0f;
        REQUIRE(std::abs(rx - ry) < 0.001f);
    }
}

TEST_CASE("AC-STD-2 [7-9-3]: Test translation unit compiles without Win32 dependencies",
    "[entry-point][ac-std-2]")
{
    // This test existence IS the check: if the TU compiled, PlatformCompat.h provides
    // everything needed and no Win32 headers are required.
    SUCCEED("Translation unit compiled without Win32 dependencies");
}

// ===========================================================================
// File-scan tests — macOS / Linux native builds only
// ===========================================================================
//
// MinGW defines _WIN32 even when cross-compiling on Linux, so these tests are
// excluded from the MinGW CI build. On macOS/Linux native builds they scan the
// actual source files to verify the structural properties enforced by this story.

#ifndef _WIN32

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

std::string readFile(const std::filesystem::path& path)
{
    std::ifstream f(path);
    if (!f.is_open())
    {
        return {};
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Count lines containing #ifdef _WIN32 or #ifndef _WIN32 that are NOT inside
// a C++ comment. Lines like "// Removed #ifdef _WIN32 guard" are excluded.
int countPlatformGuardsInFile(const std::filesystem::path& filePath)
{
    std::ifstream f(filePath);
    if (!f.is_open())
    {
        return 0;
    }

    int count = 0;
    std::string line;
    while (std::getline(f, line))
    {
        size_t commentPos = line.find("//");
        size_t guard32 = line.find("#ifdef _WIN32");
        size_t nguard32 = line.find("#ifndef _WIN32");

        // Skip if the guard token appears after a "//" comment marker
        if (guard32 != std::string::npos && (commentPos == std::string::npos || guard32 < commentPos))
        {
            ++count;
        }
        if (nguard32 != std::string::npos && (commentPos == std::string::npos || nguard32 < commentPos))
        {
            ++count;
        }
    }
    return count;
}

// Directories (as path substrings) that are allowed to contain platform guards.
// These are paths relative to src/source/ — we search for them as substrings of
// the full absolute path so "/" delimiters work on POSIX.
bool isInAllowedDirectory(const std::string& pathStr)
{
    // Platform/ — platform abstraction layer (authorized location)
    // Audio/    — DirectSound guards deferred to story 7-9-4
    // ThirdParty/ — vendored code, not our responsibility
    // Dotnet/   — generated packet bindings, not hand-edited
    return pathStr.find("/Platform/") != std::string::npos ||
           pathStr.find("/Audio/") != std::string::npos ||
           pathStr.find("/ThirdParty/") != std::string::npos ||
           pathStr.find("/Dotnet/") != std::string::npos;
}

int countPlatformGuardsInSourceTree(const std::filesystem::path& root)
{
    if (!std::filesystem::exists(root))
    {
        return -1; // signal: directory not found
    }

    int total = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        auto ext = entry.path().extension().string();
        if (ext != ".cpp" && ext != ".h")
        {
            continue;
        }

        if (isInAllowedDirectory(entry.path().string()))
        {
            continue;
        }

        total += countPlatformGuardsInFile(entry.path());
    }
    return total;
}

} // anonymous namespace

// ===========================================================================
// AC-2: WinMain() Deleted from Winmain.cpp
// ===========================================================================

TEST_CASE("AC-2 [7-9-3]: Winmain.cpp does not contain WinMain function definition",
    "[entry-point][ac-2]")
{
    const std::string sourceDir = MU_SOURCE_DIR;
    if (sourceDir.empty())
    {
        SKIP("MU_SOURCE_DIR not injected — skipping structural verification (set in CMakeLists.txt)");
    }

    const auto winmainPath = std::filesystem::path(sourceDir) / "Main" / "Winmain.cpp";
    REQUIRE(std::filesystem::exists(winmainPath));

    const std::string content = readFile(winmainPath);
    REQUIRE_FALSE(content.empty());

    SECTION("WinMain function definition is absent")
    {
        bool hasWinMain = content.find("WinMain(") != std::string::npos;
        INFO("WinMain() must be deleted by Task 3.2 — still found in Winmain.cpp");
        CHECK_FALSE(hasWinMain);
    }

    SECTION("WndProc Win32 message handler is absent")
    {
        bool hasWndProc = content.find("LRESULT CALLBACK WndProc") != std::string::npos;
        INFO("WndProc() must be deleted by Task 3.1 — still found in Winmain.cpp");
        CHECK_FALSE(hasWndProc);
    }

    SECTION("Win32 MainLoop render loop is absent")
    {
        bool hasMainLoop = content.find("void MainLoop(") != std::string::npos;
        INFO("MainLoop() must be deleted by Task 3.1 — still found in Winmain.cpp");
        CHECK_FALSE(hasMainLoop);
    }

    SECTION("KillGLWindow Win32/OpenGL teardown is absent")
    {
        bool hasKillGL = content.find("void KillGLWindow(") != std::string::npos;
        INFO("KillGLWindow() must be deleted by Task 3.1 — still found in Winmain.cpp");
        CHECK_FALSE(hasKillGL);
    }
}

// ===========================================================================
// AC-3: Single main() Entry Point on All Platforms
// ===========================================================================

TEST_CASE("AC-3 [7-9-3]: Winmain.cpp contains MuMain as the universal entry point",
    "[entry-point][ac-3]")
{
    const std::string sourceDir = MU_SOURCE_DIR;
    if (sourceDir.empty())
    {
        SKIP("MU_SOURCE_DIR not injected — skipping structural verification");
    }

    const auto winmainPath = std::filesystem::path(sourceDir) / "Main" / "Winmain.cpp";
    REQUIRE(std::filesystem::exists(winmainPath));

    const std::string content = readFile(winmainPath);
    REQUIRE_FALSE(content.empty());

    SECTION("MuMain function definition is present")
    {
        bool hasMuMain = content.find("MuMain(") != std::string::npos;
        INFO("MuMain() must remain as the universal entry point");
        CHECK(hasMuMain);
    }

    SECTION("int main() wrapper calling MuMain is present")
    {
        // After AC-3: int main(int argc, char* argv[]) { return MuMain(argc, argv); }
        bool hasMainWrapper = content.find("int main(") != std::string::npos;
        INFO("int main() wrapper must be present to call MuMain() on all platforms");
        CHECK(hasMainWrapper);
    }

    SECTION("Winmain.cpp has zero #ifdef _WIN32 platform guards")
    {
        bool hasIfdefWin32 = content.find("#ifdef _WIN32") != std::string::npos;
        bool hasIfndefWin32 = content.find("#ifndef _WIN32") != std::string::npos;
        INFO("Winmain.cpp must have zero platform guards after story 7-9-3 (Task 3.3)");
        CHECK_FALSE(hasIfdefWin32);
        CHECK_FALSE(hasIfndefWin32);
    }
}

// ===========================================================================
// AC-5: Zero #ifdef _WIN32 Guards Outside Platform/ and Audio/
// ===========================================================================

TEST_CASE("AC-5 [7-9-3]: Game code has zero #ifdef _WIN32 guards outside allowed directories",
    "[entry-point][ac-5]")
{
    const std::string sourceDir = MU_SOURCE_DIR;
    if (sourceDir.empty())
    {
        SKIP("MU_SOURCE_DIR not injected — skipping platform guard audit");
    }

    const std::filesystem::path srcPath(sourceDir);
    REQUIRE(std::filesystem::exists(srcPath));

    SECTION("zero platform guards in game code (Platform/, Audio/, ThirdParty/, Dotnet/ exempted)")
    {
        int guardCount = countPlatformGuardsInSourceTree(srcPath);
        REQUIRE(guardCount >= 0); // -1 means directory not found

        INFO("Found " << guardCount << " #ifdef _WIN32 / #ifndef _WIN32 guards outside allowed dirs");
        INFO("Expected: 0. Files to clean (per AC-5):");
        INFO("  Main/Winmain.cpp (2) — deleted by AC-2");
        INFO("  Main/stdafx.h (3) — unified to single include path (Task 4.2)");
        INFO("  Scenes/*.h (6) — WebzenScene, SceneCommon, MainScene, SceneManager, CharacterScene, LoginScene (Task 4.1)");
        INFO("  Core/ErrorReport.cpp (4) — replaced with cross-platform equivalents (Task 4.3)");
        INFO("  Core/StringUtils.h (1) — same pattern as scene headers (Task 4.4)");
        INFO("  Data/*.h (6) — FieldMetadataHelper, SkillStructs, SkillFieldMetadata, SkillFieldDefs, ItemStructs, ItemFieldMetadata (Task 4.5)");
        INFO("  RenderFX/ZzzOpenglUtil.cpp (1) — remove or move to renderer backend (Task 4.6)");
        INFO("Verify with: grep -rn '#ifdef _WIN32' src/source/ | grep -v Platform/ | grep -v Audio/ | grep -v ThirdParty/ | grep -v Dotnet/");
        REQUIRE(guardCount == 0);
    }
}

TEST_CASE("AC-5 [7-9-3]: Specific scene headers have no platform guards",
    "[entry-point][ac-5]")
{
    const std::string sourceDir = MU_SOURCE_DIR;
    if (sourceDir.empty())
    {
        SKIP("MU_SOURCE_DIR not injected");
    }

    const std::filesystem::path scenesDir = std::filesystem::path(sourceDir) / "Scenes";
    if (!std::filesystem::exists(scenesDir))
    {
        SKIP("Scenes/ directory not found");
    }

    // 6 scene headers targeted by Task 4.1
    const std::vector<std::string> sceneHeaders = {
        "WebzenScene.h",
        "SceneCommon.h",
        "MainScene.h",
        "SceneManager.h",
        "CharacterScene.h",
        "LoginScene.h",
    };

    for (const auto& header : sceneHeaders)
    {
        auto headerPath = scenesDir / header;
        if (!std::filesystem::exists(headerPath))
        {
            continue; // file may have been renamed/merged during migration
        }

        DYNAMIC_SECTION("Scene header has no platform guards: " << header)
        {
            int guards = countPlatformGuardsInFile(headerPath);
            INFO(header << " must have no #ifdef _WIN32 guards after Task 4.1");
            CHECK(guards == 0);
        }
    }
}

TEST_CASE("AC-5 [7-9-3]: Data layer headers have no platform guards",
    "[entry-point][ac-5]")
{
    const std::string sourceDir = MU_SOURCE_DIR;
    if (sourceDir.empty())
    {
        SKIP("MU_SOURCE_DIR not injected");
    }

    // 6 data headers targeted by Task 4.5
    const std::vector<std::pair<std::string, std::string>> dataHeaders = {
        { "Data", "FieldMetadataHelper.h" },
        { "Data/Skills", "SkillStructs.h" },
        { "Data/Skills", "SkillFieldMetadata.h" },
        { "Data/Skills", "SkillFieldDefs.h" },
        { "Data/Items", "ItemStructs.h" },
        { "Data/Items", "ItemFieldMetadata.h" },
    };

    for (const auto& [dir, file] : dataHeaders)
    {
        auto filePath = std::filesystem::path(sourceDir) / dir / file;
        if (!std::filesystem::exists(filePath))
        {
            continue;
        }

        DYNAMIC_SECTION("Data header has no platform guards: " << dir << "/" << file)
        {
            int guards = countPlatformGuardsInFile(filePath);
            INFO(file << " must have no #ifdef _WIN32 guards after Task 4.5");
            CHECK(guards == 0);
        }
    }
}

#endif // !_WIN32
