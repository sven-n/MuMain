#include <doctest.h>

#include "Render/Textures/TextureFiltering.h"

TEST_CASE("world folders under Data are world textures [render][mipmap]")
{
    CHECK(Render::Textures::IsWorldTexturePath(L"Data\\World1\\TileGrass01.jpg"));
    CHECK(Render::Textures::IsWorldTexturePath(L"Data\\World74\\TileRock02.jpg"));
    CHECK(Render::Textures::IsWorldTexturePath(L"Data\\Object1\\tree01.tga"));
    CHECK(Render::Textures::IsWorldTexturePath(L"Data\\Object\\stone.jpg"));
    CHECK(Render::Textures::IsWorldTexturePath(L"Data\\Player\\skin_class01.jpg"));
    CHECK(Render::Textures::IsWorldTexturePath(L"Data\\Monster\\bull.jpg"));
    CHECK(Render::Textures::IsWorldTexturePath(L"Data\\Item\\Texture\\sword01.jpg"));
    CHECK(Render::Textures::IsWorldTexturePath(L"Data\\NPC\\smith.jpg"));
    CHECK(Render::Textures::IsWorldTexturePath(L"Data\\Effect\\flare01.jpg"));
    CHECK(Render::Textures::IsWorldTexturePath(L"Data\\Skill\\arrow.tga"));
}

TEST_CASE("world texture paths ignore case and separator style [render][mipmap]")
{
    CHECK(Render::Textures::IsWorldTexturePath(L"data\\effect\\flare01.jpg"));
    CHECK(Render::Textures::IsWorldTexturePath(L"DATA/npc/smith.jpg"));
    CHECK(Render::Textures::IsWorldTexturePath(L"Data/world3/TileGrass01.jpg"));
}

TEST_CASE("everything outside the world folders is screen-space [render][mipmap]")
{
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"Data\\Interface\\newui_item_box.tga"));
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"Data\\Interface\\Cursor.tga"));
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"Data\\Logo\\logo01.jpg"));
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"Data\\Local\\Eng\\logo.jpg"));
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"Data\\WorldMap\\map.jpg"));
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"Data\\Items\\sword.jpg"));
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"Shop\\Item\\banner.jpg"));
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"/tmp/Data/Item/sword.jpg"));
}

TEST_CASE("a world folder without a file is not a texture path [render][mipmap]")
{
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L""));
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"Data"));
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"Data\\Item"));
    CHECK_FALSE(Render::Textures::IsWorldTexturePath(L"Data\\Item\\"));
}

TEST_CASE("anisotropy is clamped to what every backend supports [render][mipmap]")
{
    using Render::Textures::ClampAnisotropy;
    using Render::Textures::MaxSupportedAnisotropy;

    CHECK(ClampAnisotropy(8).value == 8);
    CHECK_FALSE(ClampAnisotropy(8).clamped);
    CHECK(ClampAnisotropy(16).value == 16);
    CHECK_FALSE(ClampAnisotropy(16).clamped);
    CHECK(ClampAnisotropy(1).value == 1);
    CHECK(ClampAnisotropy(0).value == 1);
    CHECK(ClampAnisotropy(-4).value == 1);
    CHECK_FALSE(ClampAnisotropy(-4).clamped);
    CHECK(ClampAnisotropy(64).value == MaxSupportedAnisotropy);
    CHECK(ClampAnisotropy(64).clamped);
}
