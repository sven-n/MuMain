#include <doctest.h>

#include "Core/Platform/UserFolder.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace
{
namespace fs = std::filesystem;
using Core::Platform::Seeded;

std::string ReadText(const fs::path& file)
{
    std::ifstream stream(file, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

void WriteText(const fs::path& file, const std::string& text)
{
    fs::create_directories(file.parent_path());
    std::ofstream(file, std::ios::binary) << text;
}

// A fresh folder of its own for one case.
struct ScratchFolder
{
    fs::path path = fs::temp_directory_path() / "mu-test-user-folder";
    ScratchFolder()
    {
        fs::remove_all(path);
        fs::create_directories(path);
    }
    ~ScratchFolder()
    {
        std::error_code ignored;
        fs::remove_all(path, ignored);
    }
};
} // namespace

TEST_CASE("A distributor's name comes from the shipped defaults' [Client] Name [core][user_folder]")
{
    using Core::Platform::ClientNameFromDefaults;
    CHECK(ClientNameFromDefaults("[Client]\nName=Valhalla MU\n") == "Valhalla MU");
    CHECK(ClientNameFromDefaults("[LOGIN]\nName=Other\n[client]\r\n name = Server-2_x \r\n") == "Server-2_x");
    CHECK_FALSE(ClientNameFromDefaults("[Render]\nVSync=1\n"));
    CHECK_FALSE(ClientNameFromDefaults(""));
}

TEST_CASE("No name, or one that isn't a plain folder name, leaves it to the install [core][user_folder]")
{
    using Core::Platform::ClientNameFromDefaults;
    CHECK_FALSE(ClientNameFromDefaults("[Client]\nName=\n"));
    CHECK_FALSE(ClientNameFromDefaults("[Client]\nName=..\n"));
    CHECK_FALSE(ClientNameFromDefaults("[Client]\nName=a/b\n"));
    CHECK_FALSE(ClientNameFromDefaults("[Client]\nName=a\\b\n"));
    CHECK_FALSE(ClientNameFromDefaults("[Client]\nName=C:x\n"));
    CHECK_FALSE(ClientNameFromDefaults("[Client]\nName=.hidden\n"));
}

TEST_CASE("An install without a name gets its folder's name and a hash of its path [core][user_folder]")
{
    using Core::Platform::InstallFolderName;
    // The values scripts/wayland-diag.sh computes for the same paths.
    CHECK(InstallFolderName(fs::path(u8"/games/My MU")) == "My MU-12d21476");
    CHECK(InstallFolderName(fs::path(u8"/games/My MU/")) == "My MU-12d21476");
    CHECK(InstallFolderName(fs::path(u8"/games/Мой MU")) == "______ MU-89365efb");
    CHECK(InstallFolderName(fs::path(u8"/")) == "MuMain-860189fe");
    // Two installs of the same folder name stay apart.
    CHECK(InstallFolderName(fs::path(u8"/a/MU")) != InstallFolderName(fs::path(u8"/b/MU")));
}

TEST_CASE("The variable wins, then the target's folder, then the working directory [core][user_folder]")
{
    using Core::Platform::ResolveUserFolder;
    const fs::path target = "target";
    const fs::path working = "working";

    CHECK(ResolveUserFolder("chosen", target, working).folder == fs::path("chosen"));
    CHECK_FALSE(ResolveUserFolder("chosen", target, working).fallback);
    CHECK(ResolveUserFolder("", target, working).folder == target);
    CHECK(ResolveUserFolder(nullptr, target, working).folder == target);

    const auto fallback = ResolveUserFolder(nullptr, {}, working);
    CHECK(fallback.folder == working);
    CHECK(fallback.fallback);
}

TEST_CASE("The log shows the home folder as ~ [core][user_folder]")
{
    using Core::Platform::DisplayPath;
    const fs::path home = fs::path("home") / "player";
    CHECK(DisplayPath(home / "MuMain" / "MuMain", home) == (fs::path("~") / "MuMain" / "MuMain").string());
    CHECK(DisplayPath(fs::path("home") / "player2" / "x", home) == (fs::path("home") / "player2" / "x").string());
    CHECK(DisplayPath(fs::path("elsewhere"), fs::path()) == "elsewhere");
}

TEST_CASE("A missing user file is copied once from the old one [core][user_folder]")
{
    const ScratchFolder scratch;
    const fs::path user = scratch.path / "user" / "config.ini";
    const fs::path legacy = scratch.path / "client" / "config.ini";
    WriteText(legacy, "[Window]\nWidth=1920\n");

    CHECK(Core::Platform::SeedUserFile(user, legacy, std::string("[Window]\nWidth=1024\n")) == Seeded::FromLegacy);
    CHECK(ReadText(user) == "[Window]\nWidth=1920\n");

    // The player's own changes survive the next start, whatever the old file or the defaults say.
    WriteText(user, "[Window]\nWidth=800\n");
    WriteText(legacy, "[Window]\nWidth=640\n");
    CHECK(Core::Platform::SeedUserFile(user, legacy, std::string("[Window]\nWidth=1024\n")) == Seeded::Existing);
    CHECK(ReadText(user) == "[Window]\nWidth=800\n");
}

TEST_CASE("Without an old file the user file comes from the shipped defaults [core][user_folder]")
{
    const ScratchFolder scratch;
    const fs::path user = scratch.path / "user" / "config.ini";

    CHECK(Core::Platform::SeedUserFile(user, scratch.path / "missing.ini", std::string("[Audio]\nSoundVolume=5\n")) ==
          Seeded::FromDefaults);
    CHECK(ReadText(user) == "[Audio]\nSoundVolume=5\n");

    const fs::path macros = scratch.path / "user" / "Macro.txt";
    CHECK(Core::Platform::SeedUserFile(macros, {}, std::nullopt) == Seeded::Nothing);
    CHECK_FALSE(fs::exists(macros));
}
