#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

// The OZT files the game loads for .tga textures: a TGA file with 4 more bytes
// in front. Like the original client, the game reads only uncompressed 32 bit
// images, with the pixels right after the header and the bottom row first.
namespace Render::Textures::Ozt
{
// The 4 bytes in front and the 18 bytes of the TGA header.
inline constexpr std::size_t HeaderSize = 4 + 18;
inline constexpr std::size_t BytesPerPixel = 4;

enum class Status
{
    Valid,
    // The file ends inside the header or before the last pixel.
    TooSmall,
    // Not 32 bit, no pixels, or larger than the largest texture.
    InvalidFormat,
};

struct Header
{
    Status status = Status::InvalidFormat;
    int width = 0;
    int height = 0;
    int bitsPerPixel = 0;
};

// Reads the size and depth of an OZT file and checks that the game can read
// it: 32 bit, at most maxWidth x maxHeight, and every pixel in the file. The
// size and depth are filled in for an invalid format too, for the log.
Header ReadHeader(std::span<const std::uint8_t> file, int maxWidth, int maxHeight);

// Copies the pixels of a valid file into an RGBA texture of textureWidth
// pixels per row, top row first. The texture holds at least textureWidth x
// header.height pixels. Copies nothing when the header is not valid.
void CopyPixels(std::span<const std::uint8_t> file, const Header& header, std::span<std::uint8_t> texture,
                int textureWidth);
} // namespace Render::Textures::Ozt
