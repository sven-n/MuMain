#include "Render/Terrain/TerrainFileFormat.h"

#include "Render/Models/BmdModulusCipher.h"
#include "Render/Terrain/MapFileCipher.h"

#include <cstring>

namespace Render::Terrain::FileFormat
{
namespace
{
constexpr std::size_t TagSize = 4;
constexpr std::uint8_t TagVersion = 1;
constexpr std::size_t TileCount = TerrainSide * TerrainSide;

// What an 8-bit height map holds besides one byte a tile: the .OZB prefix and the BMP headers with the palette.
constexpr std::size_t HeightMapOverhead = 4 + 1080;

constexpr std::size_t OzbPrefixSize = 4;
constexpr std::size_t BmpHeaderSize = 54;
constexpr std::size_t BmpPixelOffsetAt = 10;
constexpr std::size_t BmpWidthAt = 18;
constexpr std::size_t BmpHeightAt = 22;
constexpr std::size_t BmpBitsAt = 28;
constexpr std::uint16_t LightBits = 24;
constexpr std::size_t BytesPerPixel = 3;
constexpr float MaxChannel = 255.f;

bool HasTerrainTag(const std::vector<std::uint8_t>& file)
{
    if (file.size() <= TagSize || file[3] != TagVersion)
        return false;
    const bool map = file[0] == 'M' && file[1] == 'A' && file[2] == 'P';
    const bool att = file[0] == 'A' && file[1] == 'T' && file[2] == 'T';
    return map || att;
}

template <class T> T ReadLittleEndian(const std::uint8_t* at)
{
    T value{};
    std::memcpy(&value, at, sizeof(value));
    return value;
}
} // namespace

std::vector<std::uint8_t> DecryptTerrainFile(const std::vector<std::uint8_t>& file)
{
    if (!HasTerrainTag(file))
    {
        std::vector<std::uint8_t> decrypted(file.size());
        MapFileDecrypt(decrypted.data(), file.data(), static_cast<int>(file.size()));
        return decrypted;
    }

    std::vector<std::uint8_t> payload(file.begin() + TagSize, file.end());
    if (!Render::Models::BmdModulusCipher::DecryptInPlace(payload.data(), payload.size()))
        return {};
    return {payload.begin() + Render::Models::BmdModulusCipher::BodyOffset, payload.end()};
}

std::size_t ObjectRecordSize(std::uint8_t version)
{
    constexpr std::array<std::size_t, 6> RecordSizes = {30, 32, 33, 45, 46, 54};
    return version < RecordSizes.size() ? RecordSizes[version] : 0;
}

bool IsHeightMap24Bit(std::size_t fileSize)
{
    return fileSize > HeightMapOverhead + TileCount;
}

std::vector<std::array<float, 3>> ReadLightBitmap(std::span<const std::uint8_t> content)
{
    if (content.size() < OzbPrefixSize + BmpHeaderSize)
        return {};
    const std::uint8_t* bmp = content.data() + OzbPrefixSize;
    const auto pixelOffset = ReadLittleEndian<std::uint32_t>(bmp + BmpPixelOffsetAt);
    const auto width = ReadLittleEndian<std::int32_t>(bmp + BmpWidthAt);
    const auto height = ReadLittleEndian<std::int32_t>(bmp + BmpHeightAt);
    const auto bits = ReadLittleEndian<std::uint16_t>(bmp + BmpBitsAt);
    const bool terrainSized =
        width == static_cast<std::int32_t>(TerrainSide) && height == static_cast<std::int32_t>(TerrainSide);
    if (!terrainSized || bits != LightBits || content.size() < OzbPrefixSize + pixelOffset + TileCount * BytesPerPixel)
        return {};

    const std::uint8_t* pixels = bmp + pixelOffset;
    std::vector<std::array<float, 3>> light(TileCount);
    for (std::size_t i = 0; i < TileCount; ++i)
    {
        const std::uint8_t* bgr = pixels + i * BytesPerPixel;
        light[i] = {bgr[2] / MaxChannel, bgr[1] / MaxChannel, bgr[0] / MaxChannel};
    }
    return light;
}
} // namespace Render::Terrain::FileFormat
