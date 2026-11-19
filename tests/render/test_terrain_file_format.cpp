#include <doctest.h>

#include "BmdModulusTestEncryption.h"
#include "Render/Terrain/MapFileCipher.h"
#include "Render/Terrain/TerrainFileFormat.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

namespace
{
using namespace Render::Terrain::FileFormat;

constexpr std::size_t TileCount = TerrainSide * TerrainSide;

std::vector<std::uint8_t> Tagged(const char (&tag)[4], const std::vector<std::uint8_t>& payload)
{
    std::vector<std::uint8_t> file = {static_cast<std::uint8_t>(tag[0]), static_cast<std::uint8_t>(tag[1]),
                                      static_cast<std::uint8_t>(tag[2]), 1};
    file.insert(file.end(), payload.begin(), payload.end());
    return file;
}

template <class T> void Write(std::vector<std::uint8_t>& data, std::size_t at, T value)
{
    std::memcpy(data.data() + at, &value, sizeof(value));
}

// A TerrainLight.OZB: the 4-byte prefix, a BMP header and 24-bit pixels whose blue, green and red follow the tile.
std::vector<std::uint8_t> LightBitmap(std::uint16_t bits = 24,
                                      std::int32_t side = static_cast<std::int32_t>(TerrainSide))
{
    constexpr std::size_t Prefix = 4;
    constexpr std::uint32_t PixelOffset = 54;
    std::vector<std::uint8_t> file(Prefix + PixelOffset + TileCount * 3);
    Write(file, Prefix + 10, PixelOffset);
    Write(file, Prefix + 18, side);
    Write(file, Prefix + 22, side);
    Write(file, Prefix + 28, bits);
    for (std::size_t i = 0; i < TileCount; ++i)
    {
        std::uint8_t* bgr = file.data() + Prefix + PixelOffset + i * 3;
        bgr[0] = static_cast<std::uint8_t>(i);
        bgr[1] = static_cast<std::uint8_t>(i >> 8);
        bgr[2] = 255;
    }
    return file;
}
} // namespace

TEST_CASE("a classic terrain file decrypts with the map file cipher [render][terrain]")
{
    const std::vector<std::uint8_t> plain = BmdModulusTest::MakeBody(1000);
    std::vector<std::uint8_t> file(plain.size());
    MapFileEncrypt(file.data(), plain.data(), static_cast<int>(plain.size()));

    CHECK(DecryptTerrainFile(file) == plain);
}

TEST_CASE("a later season's tagged .map or .att decrypts with the version 14 cipher [render][terrain]")
{
    const std::vector<std::uint8_t> body = BmdModulusTest::MakeBody(3000);
    constexpr std::size_t Tea = 0;
    constexpr std::size_t Rc5 = 3;
    const std::vector<std::uint8_t> payload = BmdModulusTest::Encrypt(Tea, Rc5, body);

    CHECK(DecryptTerrainFile(Tagged("MAP", payload)) == body);
    CHECK(DecryptTerrainFile(Tagged("ATT", payload)) == body);
}

TEST_CASE("a tagged terrain file too short for the cipher's keys gives no content [render][terrain]")
{
    CHECK(DecryptTerrainFile(Tagged("MAP", std::vector<std::uint8_t>(10))).empty());
}

TEST_CASE("an object record's size follows the object file's version [render][terrain]")
{
    CHECK(ObjectRecordSize(0) == 30);
    CHECK(ObjectRecordSize(1) == 32);
    CHECK(ObjectRecordSize(2) == 33);
    CHECK(ObjectRecordSize(3) == 45);
    CHECK(ObjectRecordSize(4) == 46);
    CHECK(ObjectRecordSize(5) == 54);
    CHECK(ObjectRecordSize(6) == 0);
}

TEST_CASE("a height map holds 24-bit heights when its file is larger than an 8-bit one [render][terrain]")
{
    // 8-bit: the 4-byte prefix, 1080 bytes of BMP headers and palette, a byte a tile (every classic world but three).
    CHECK_FALSE(IsHeightMap24Bit(66620));
    // 24-bit: the prefix, the 54-byte BMP header and three bytes a tile (worlds 43, 64 and 67, and later seasons').
    CHECK(IsHeightMap24Bit(196666));
}

TEST_CASE("a light bitmap gives red, green and blue from 0 to 1 per tile [render][terrain]")
{
    const std::vector<std::array<float, 3>> light = ReadLightBitmap(LightBitmap());

    REQUIRE(light.size() == TileCount);
    CHECK(light[0][0] == doctest::Approx(1.f));
    CHECK(light[0][2] == doctest::Approx(0.f));
    constexpr std::size_t Tile = 0x1234;
    CHECK(light[Tile][1] == doctest::Approx(0x12 / 255.f));
    CHECK(light[Tile][2] == doctest::Approx(0x34 / 255.f));
}

TEST_CASE("a light bitmap of another depth or size, or cut short, gives no light [render][terrain]")
{
    CHECK(ReadLightBitmap(LightBitmap(8)).empty());
    CHECK(ReadLightBitmap(LightBitmap(24, 128)).empty());
    std::vector<std::uint8_t> cut = LightBitmap();
    cut.resize(cut.size() - 1);
    CHECK(ReadLightBitmap(cut).empty());
    CHECK(ReadLightBitmap({}).empty());
}
