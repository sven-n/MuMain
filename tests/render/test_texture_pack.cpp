#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <doctest.h>

#include "Render/Textures/TexturePack.h"
#include "Render/Textures/TexturePackFiles.h"

namespace
{
namespace TexturePack = Render::Textures::TexturePack;
using Render::Textures::TexturePackFiles;

// A fresh folder under the temp directory, removed when the test ends.
struct TempFolder
{
    std::filesystem::path path = std::filesystem::temp_directory_path() / "mu_texture_pack_tests";

    TempFolder()
    {
        std::filesystem::remove_all(path);
        std::filesystem::create_directories(path);
    }
    ~TempFolder() { std::filesystem::remove_all(path); }
};

void WriteFile(const std::filesystem::path& path)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path) << "texture";
}

// A pack folder with a texture in World1 and one in Interface.
struct PackFolder
{
    TempFolder folder;

    PackFolder()
    {
        WriteFile(folder.path / "World1" / "TileGrass01.OZJ");
        WriteFile(folder.path / "Interface" / "newui_button.ozt");
    }
};
} // namespace

TEST_CASE("a pack name is a plain folder name [render][texture_pack]")
{
    CHECK(TexturePack::IsValidName(L"upscaled"));
    CHECK(TexturePack::IsValidName(L"HD_textures-2"));
    CHECK_FALSE(TexturePack::IsValidName(L""));
    CHECK_FALSE(TexturePack::IsValidName(L".."));
    CHECK_FALSE(TexturePack::IsValidName(L"a/b"));
    CHECK_FALSE(TexturePack::IsValidName(L"a\\b"));
    CHECK_FALSE(TexturePack::IsValidName(L"my pack"));
}

TEST_CASE("a pack's folder is in Data/TexturePacks [render][texture_pack]")
{
    CHECK(TexturePack::PackFolder(L"upscaled") == L"Data\\TexturePacks\\upscaled");
}

TEST_CASE("pack names compare in any case [render][texture_pack]")
{
    CHECK(TexturePack::SameName(L"upscaled", L"Upscaled"));
    CHECK(TexturePack::SameName(L"HD-2", L"hd-2"));
    CHECK_FALSE(TexturePack::SameName(L"upscaled", L"upscaled2"));
    CHECK_FALSE(TexturePack::SameName(L"upscaled", L""));
}

TEST_CASE("the packs are the folders with a valid name, sorted [render][texture_pack]")
{
    const TempFolder folder;
    std::filesystem::create_directory(folder.path / "upscaled");
    std::filesystem::create_directory(folder.path / "Classic");
    std::filesystem::create_directory(folder.path / "not valid");
    std::ofstream(folder.path / "readme.txt") << "a file, not a pack";

    CHECK(TexturePack::ListPacks(folder.path) == std::vector<std::wstring>{L"Classic", L"upscaled"});
}

TEST_CASE("a missing packs folder has no packs [render][texture_pack]")
{
    CHECK(TexturePack::ListPacks(std::filesystem::temp_directory_path() / "mu_no_such_folder").empty());
}

TEST_CASE("a file in Data has its copy in the pack [render][texture_pack]")
{
    const PackFolder pack;
    const TexturePackFiles files = TexturePackFiles::List(pack.folder.path);

    REQUIRE(files.Count() == 2);
    const std::filesystem::path* copy = files.Find(L"Data\\World1\\TileGrass01.OZJ");
    REQUIRE(copy != nullptr);
    CHECK(*copy == pack.folder.path / "World1" / "TileGrass01.OZJ");
}

TEST_CASE("a copy matches in any case and with either separator [render][texture_pack]")
{
    const PackFolder pack;
    const TexturePackFiles files = TexturePackFiles::List(pack.folder.path);

    CHECK(files.Find(L"data\\world1\\tilegrass01.ozj") != nullptr);
    CHECK(files.Find(L"DATA/World1/TileGrass01.OZJ") != nullptr);
    const std::filesystem::path* copy = files.Find(L"Data\\Interface\\NewUI_Button.OZT");
    REQUIRE(copy != nullptr);
    CHECK(*copy == pack.folder.path / "Interface" / "newui_button.ozt");
}

TEST_CASE("a file the pack lacks, or one outside Data, has no copy [render][texture_pack]")
{
    const PackFolder pack;
    const TexturePackFiles files = TexturePackFiles::List(pack.folder.path);

    CHECK(files.Find(L"Data\\World1\\TileGrass02.OZJ") == nullptr);
    CHECK(files.Find(L"Data\\World2\\TileGrass01.OZJ") == nullptr);
    CHECK(files.Find(L"Data2\\World1\\TileGrass01.OZJ") == nullptr);
    CHECK(files.Find(L"Database\\World1\\TileGrass01.OZJ") == nullptr);
    CHECK(files.Find(L"World1\\TileGrass01.OZJ") == nullptr);
    CHECK(files.Find(L"Data") == nullptr);
}

TEST_CASE("only OZJ and OZT files are listed [render][texture_pack]")
{
    const TempFolder folder;
    WriteFile(folder.path / "World1" / "TileGrass01.OZJ");
    WriteFile(folder.path / "World1" / "TileGrass01.jpg");
    WriteFile(folder.path / "readme.txt");

    const TexturePackFiles files = TexturePackFiles::List(folder.path);
    CHECK(files.Count() == 1);
    CHECK(files.Find(L"Data\\World1\\TileGrass01.jpg") == nullptr);
}

TEST_CASE("no pack, or a missing pack folder, has no files [render][texture_pack]")
{
    CHECK(TexturePackFiles{}.Empty());
    CHECK(TexturePackFiles{}.Find(L"Data\\World1\\TileGrass01.OZJ") == nullptr);
    CHECK(TexturePackFiles::List(std::filesystem::temp_directory_path() / "mu_no_such_folder").Empty());
}
