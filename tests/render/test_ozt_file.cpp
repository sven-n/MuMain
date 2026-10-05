#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <doctest.h>

#include "Render/Textures/OztFile.h"

namespace
{
namespace Ozt = Render::Textures::Ozt;

// The largest texture of CGlobalBitmap (MAX_WIDTH, MAX_HEIGHT).
constexpr int MaxSize = 1024;

// The TGA header starts after the 4 bytes in front.
constexpr std::size_t ImageTypeOffset = 4 + 2;
constexpr std::size_t WidthOffset = 4 + 12;
constexpr std::size_t HeightOffset = 4 + 14;
constexpr std::size_t DepthOffset = 4 + 16;
constexpr std::uint8_t UncompressedTrueColor = 2;

const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;

void WriteUInt16(std::vector<std::uint8_t>& file, std::size_t offset, int value)
{
    file[offset] = static_cast<std::uint8_t>(value & 0xFF);
    file[offset + 1] = static_cast<std::uint8_t>(value >> 8);
}

// A whole OZT file that ends with its last pixel. Pixel i is stored as the
// bytes 4i, 4i + 1, 4i + 2 and 4i + 3 (BGRA, bottom row first), cut to 8 bits.
std::vector<std::uint8_t> MakeOzt(int width, int height, int bitsPerPixel = 32)
{
    const std::size_t pixelBytes =
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * Ozt::BytesPerPixel;
    std::vector<std::uint8_t> file(Ozt::HeaderSize + pixelBytes, 0);
    file[ImageTypeOffset] = UncompressedTrueColor;
    WriteUInt16(file, WidthOffset, width);
    WriteUInt16(file, HeightOffset, height);
    file[DepthOffset] = static_cast<std::uint8_t>(bitsPerPixel);
    for (std::size_t i = 0; i < pixelBytes; ++i)
    {
        file[Ozt::HeaderSize + i] = static_cast<std::uint8_t>(i);
    }
    return file;
}

Ozt::Status ReadStatus(const std::vector<std::uint8_t>& file)
{
    return Ozt::ReadHeader(file, MaxSize, MaxSize).status;
}

bool IsOztFile(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return extension == ".ozt";
}

std::vector<std::uint8_t> ReadFile(const std::filesystem::path& path)
{
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(std::filesystem::file_size(path)));
    std::ifstream file(path, std::ios::binary);
    file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return bytes;
}
} // namespace

TEST_CASE("an OZT file that ends with its last pixel is valid [render][ozt_file]")
{
    const Ozt::Header header = Ozt::ReadHeader(MakeOzt(256, 256), MaxSize, MaxSize);

    CHECK(header.status == Ozt::Status::Valid);
    CHECK(header.width == 256);
    CHECK(header.height == 256);
    CHECK(header.bitsPerPixel == 32);
}

TEST_CASE("bytes after the pixels keep an OZT file valid [render][ozt_file]")
{
    // Most shipped files end with the 26 bytes of a TGA footer.
    std::vector<std::uint8_t> file = MakeOzt(2, 2);
    file.resize(file.size() + 26);

    CHECK(ReadStatus(file) == Ozt::Status::Valid);
}

TEST_CASE("an OZT file cut off in the header is too small [render][ozt_file]")
{
    const std::vector<std::uint8_t> whole = MakeOzt(2, 2);
    for (const std::size_t size : {std::size_t{0}, std::size_t{4}, std::size_t{18}, Ozt::HeaderSize - 1})
    {
        CAPTURE(size);
        const std::vector<std::uint8_t> file(whole.begin(), whole.begin() + static_cast<std::ptrdiff_t>(size));
        CHECK(ReadStatus(file) == Ozt::Status::TooSmall);
    }
}

TEST_CASE("an OZT file cut off in the pixels is too small [render][ozt_file]")
{
    // The header says 256 x 256 at 32 bit: 256 KiB of pixels.
    std::vector<std::uint8_t> file = MakeOzt(256, 256);
    file.pop_back();
    CHECK(ReadStatus(file) == Ozt::Status::TooSmall);

    file.resize(Ozt::HeaderSize);
    CHECK(ReadStatus(file) == Ozt::Status::TooSmall);
}

TEST_CASE("OZT files the game cannot read have an invalid format [render][ozt_file]")
{
    const Ozt::Header depth24 = Ozt::ReadHeader(MakeOzt(2, 3, 24), MaxSize, MaxSize);
    CHECK(depth24.status == Ozt::Status::InvalidFormat);
    CHECK(depth24.bitsPerPixel == 24);
    CHECK(depth24.width == 2);
    CHECK(depth24.height == 3);

    CHECK(ReadStatus(MakeOzt(0, 2)) == Ozt::Status::InvalidFormat);
    CHECK(ReadStatus(MakeOzt(2, 0)) == Ozt::Status::InvalidFormat);
    CHECK(ReadStatus(MakeOzt(MaxSize + 1, 1)) == Ozt::Status::InvalidFormat);
    CHECK(ReadStatus(MakeOzt(1, MaxSize + 1)) == Ozt::Status::InvalidFormat);
}

TEST_CASE("the format of an OZT file is checked before its pixels [render][ozt_file]")
{
    // The log names the depth of a cut off 24 bit file, as before.
    std::vector<std::uint8_t> file = MakeOzt(2, 2, 24);
    file.resize(Ozt::HeaderSize);

    CHECK(ReadStatus(file) == Ozt::Status::InvalidFormat);
}

TEST_CASE("the largest OZT texture is valid when it is whole [render][ozt_file]")
{
    std::vector<std::uint8_t> file = MakeOzt(MaxSize, MaxSize);
    CHECK(ReadStatus(file) == Ozt::Status::Valid);

    file.pop_back();
    CHECK(ReadStatus(file) == Ozt::Status::TooSmall);
}

TEST_CASE("OZT pixels are copied top row first as RGBA [render][ozt_file]")
{
    // 3 x 2 pixels into a texture 4 pixels wide, the next power of two.
    const std::vector<std::uint8_t> file = MakeOzt(3, 2);
    const Ozt::Header header = Ozt::ReadHeader(file, MaxSize, MaxSize);
    REQUIRE(header.status == Ozt::Status::Valid);

    constexpr int TextureWidth = 4;
    std::vector<std::uint8_t> texture(static_cast<std::size_t>(TextureWidth) * 2 * Ozt::BytesPerPixel, 0);
    Ozt::CopyPixels(file, header, texture, TextureWidth);

    // The file's second row (pixels 3 to 5) is the texture's top row; the
    // padding pixel stays as it was.
    const std::vector<std::uint8_t> topRow = {14, 13, 12, 15, 18, 17, 16, 19, 22, 21, 20, 23, 0, 0, 0, 0};
    const std::vector<std::uint8_t> bottomRow = {2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 0, 0, 0, 0};
    const auto rowBytes = static_cast<std::ptrdiff_t>(TextureWidth * Ozt::BytesPerPixel);
    CHECK(std::vector<std::uint8_t>(texture.begin(), texture.begin() + rowBytes) == topRow);
    CHECK(std::vector<std::uint8_t>(texture.begin() + rowBytes, texture.end()) == bottomRow);
}

TEST_CASE("nothing is copied from an OZT file that is too small [render][ozt_file]")
{
    std::vector<std::uint8_t> file = MakeOzt(2, 2);
    file.pop_back();
    const Ozt::Header header = Ozt::ReadHeader(file, MaxSize, MaxSize);

    std::vector<std::uint8_t> texture(2 * 2 * Ozt::BytesPerPixel, 0);
    Ozt::CopyPixels(file, header, texture, 2);

    CHECK(texture == std::vector<std::uint8_t>(texture.size(), 0));
}

// Every texture the game ships keeps loading: the size check turns away only
// files that end before their last pixel. A file of another depth stays an
// invalid format, as it always was.
TEST_CASE("no shipped OZT file is too small [render][ozt_file]")
{
    int checked = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(DataDirectory))
    {
        if (!entry.is_regular_file() || !IsOztFile(entry.path()))
        {
            continue;
        }

        const std::string name = entry.path().string();
        CHECK_MESSAGE(ReadStatus(ReadFile(entry.path())) != Ozt::Status::TooSmall, name);
        ++checked;
    }
    CHECK(checked > 0);
}
