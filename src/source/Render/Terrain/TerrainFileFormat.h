#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

// The formats of a world's terrain files, as classic clients and later seasons' clients write them. Pure functions
// over file contents, so the loaders stay about loading.
namespace Render::Terrain::FileFormat
{
// The side of a terrain in tiles.
constexpr std::size_t TerrainSide = 256;

// The content of a .map or .att file. Later seasons' files start with a 4-byte tag ("MAP" or "ATT", then 1) and use
// the two-stage cipher of version 14 models; classic files are XOR-encrypted whole. Empty when a tagged file is too
// short to decrypt.
std::vector<std::uint8_t> DecryptTerrainFile(const std::vector<std::uint8_t>& file);

// The size of one object record in a decrypted .obj file of the given version (its first byte). Later versions
// append fields; the leading 30 bytes (type, position, angle, scale) are the same in all. 0 for an unknown version.
std::size_t ObjectRecordSize(std::uint8_t version);

// Whether a TerrainHeight.OZB of this size holds 24-bit heights (three bytes a tile) rather than 8-bit ones.
bool IsHeightMap24Bit(std::size_t fileSize);

// The light map of a TerrainLight.OZB: a 4-byte prefix, then a 24-bit BMP of the terrain's size whose rows are in
// the light map's order and whose pixels are blue, green, red. Red, green and blue from 0 to 1 per tile; empty when
// the content is not such a bitmap.
std::vector<std::array<float, 3>> ReadLightBitmap(std::span<const std::uint8_t> content);
} // namespace Render::Terrain::FileFormat
