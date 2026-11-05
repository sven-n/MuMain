#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <doctest.h>

#include "Render/Textures/AlphaTestHoles.h"
#include "Render/Textures/OztFile.h"

namespace
{
using Render::Textures::HasAlphaTestHoles;
namespace Ozt = Render::Textures::Ozt;

constexpr std::size_t BytesPerPixel = 4;
constexpr std::uint8_t Opaque = 255;
// The largest texture of CGlobalBitmap (MAX_WIDTH, MAX_HEIGHT).
constexpr int MaxSize = 1024;

const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;

// An RGBA buffer `rowPixels` wide, every pixel opaque.
std::vector<std::uint8_t> OpaqueImage(int rowPixels, int height)
{
    return std::vector<std::uint8_t>(static_cast<std::size_t>(rowPixels) * height * BytesPerPixel, Opaque);
}

void SetAlpha(std::vector<std::uint8_t>& image, int rowPixels, int x, int y, std::uint8_t alpha)
{
    image[(static_cast<std::size_t>(y) * rowPixels + x) * BytesPerPixel + 3] = alpha;
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

TEST_CASE("An image has holes when the alpha test drops one of its pixels [render][alpha_test_holes]")
{
    std::vector<std::uint8_t> image = OpaqueImage(4, 2);
    CHECK_FALSE(HasAlphaTestHoles(image, 4, 2, 4));

    // 64 / 255 is above the 0.25 cutoff, 63 / 255 is not.
    SetAlpha(image, 4, 3, 1, 64);
    CHECK_FALSE(HasAlphaTestHoles(image, 4, 2, 4));
    SetAlpha(image, 4, 3, 1, Render::Textures::AlphaTestDropsAtOrBelow);
    CHECK(HasAlphaTestHoles(image, 4, 2, 4));
}

TEST_CASE("The padding of a power-of-two buffer is no hole [render][alpha_test_holes]")
{
    // A 3 x 2 image in a buffer 4 pixels wide and 4 rows high, its padding transparent as CreateBitmapStorage
    // leaves it.
    std::vector<std::uint8_t> buffer(static_cast<std::size_t>(4) * 4 * BytesPerPixel, 0);
    for (int y = 0; y < 2; ++y)
    {
        for (int x = 0; x < 3; ++x)
        {
            SetAlpha(buffer, 4, x, y, Opaque);
        }
    }
    CHECK_FALSE(HasAlphaTestHoles(buffer, 3, 2, 4));
    SetAlpha(buffer, 4, 2, 1, 0);
    CHECK(HasAlphaTestHoles(buffer, 3, 2, 4));
}

TEST_CASE("A buffer too small for the image, or no image, has no holes [render][alpha_test_holes]")
{
    const std::vector<std::uint8_t> image(BytesPerPixel, 0);
    CHECK_FALSE(HasAlphaTestHoles(image, 2, 2, 2));
    CHECK_FALSE(HasAlphaTestHoles(image, 0, 1, 1));
    CHECK_FALSE(HasAlphaTestHoles(image, 2, 1, 1));
}

// Most item textures of the .tga kind have holes (the cut-outs of wings, sets and capes); a few are solid.
TEST_CASE("The shipped item textures with alpha have holes or none [render][alpha_test_holes]")
{
    int withHoles = 0;
    int withoutHoles = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(DataDirectory / "Item"))
    {
        if (!entry.is_regular_file() || !IsOztFile(entry.path()))
        {
            continue;
        }
        const std::vector<std::uint8_t> file = ReadFile(entry.path());
        const Ozt::Header header = Ozt::ReadHeader(file, MaxSize, MaxSize);
        if (header.status != Ozt::Status::Valid)
        {
            continue;
        }
        std::vector<std::uint8_t> pixels(static_cast<std::size_t>(header.width) * header.height * BytesPerPixel);
        Ozt::CopyPixels(file, header, pixels, header.width);
        if (HasAlphaTestHoles(pixels, header.width, header.height, header.width))
        {
            ++withHoles;
        }
        else
        {
            ++withoutHoles;
        }
    }
    MESSAGE("item textures with holes: ", withHoles, ", without: ", withoutHoles);
    CHECK(withHoles > 0);
    CHECK(withoutHoles > 0);
}
