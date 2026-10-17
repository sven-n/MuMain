#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <doctest.h>

#include "Render/Textures/TexturePack.h"

namespace
{
namespace TexturePack = Render::Textures::TexturePack;

std::wstring PathInPack(const std::wstring& path, const std::wstring& pack = L"upscaled")
{
    return TexturePack::PathInPack(path, pack).value_or(L"");
}

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
} // namespace

TEST_CASE("a file in Data has its copy in the pack folder [render][texture_pack]")
{
    CHECK(PathInPack(L"Data\\World1\\TileGrass01.OZJ") == L"Data\\TexturePacks\\upscaled\\World1\\TileGrass01.OZJ");
    CHECK(PathInPack(L"Data\\Player\\hair_r.OZJ", L"My_Pack-2") == L"Data\\TexturePacks\\My_Pack-2\\Player\\hair_r.OZJ");
}

TEST_CASE("a path with forward slashes keeps the rest of its spelling [render][texture_pack]")
{
    CHECK(PathInPack(L"Data/Interface/RmlUi/frame.OZT") == L"Data\\TexturePacks\\upscaled/Interface/RmlUi/frame.OZT");
}

TEST_CASE("the Data folder matches in any case [render][texture_pack]")
{
    CHECK(PathInPack(L"data\\Item\\Sword01.OZJ") == L"Data\\TexturePacks\\upscaled\\Item\\Sword01.OZJ");
    CHECK(PathInPack(L"DATA\\Item\\Sword01.OZJ") == L"Data\\TexturePacks\\upscaled\\Item\\Sword01.OZJ");
}

TEST_CASE("a path outside Data has no copy in a pack [render][texture_pack]")
{
    CHECK_FALSE(TexturePack::PathInPack(L"Data2\\Object1\\Tree01.OZJ", L"upscaled").has_value());
    CHECK_FALSE(TexturePack::PathInPack(L"Database\\Tree01.OZJ", L"upscaled").has_value());
    CHECK_FALSE(TexturePack::PathInPack(L"Interface\\Foo.OZJ", L"upscaled").has_value());
    CHECK_FALSE(TexturePack::PathInPack(L"Data", L"upscaled").has_value());
}

TEST_CASE("a pack name is a plain folder name [render][texture_pack]")
{
    CHECK(TexturePack::IsValidName(L"upscaled"));
    CHECK(TexturePack::IsValidName(L"HD_textures-2"));
    CHECK_FALSE(TexturePack::IsValidName(L""));
    CHECK_FALSE(TexturePack::IsValidName(L".."));
    CHECK_FALSE(TexturePack::IsValidName(L"a/b"));
    CHECK_FALSE(TexturePack::IsValidName(L"a\\b"));
    CHECK_FALSE(TexturePack::IsValidName(L"my pack"));
    CHECK_FALSE(TexturePack::PathInPack(L"Data\\Item\\Sword01.OZJ", L"..").has_value());
    CHECK_FALSE(TexturePack::PathInPack(L"Data\\Item\\Sword01.OZJ", L"").has_value());
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
