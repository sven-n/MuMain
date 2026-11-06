#include <doctest.h>

#include "Render/PostProcessing/ColorGradingTable.h"

#include <SDL3/SDL.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

using Render::PostProcessing::GradingTableBytesPerTexel;
using Render::PostProcessing::GradingTableHeight;
using Render::PostProcessing::GradingTableLoad;
using Render::PostProcessing::GradingTableSize;
using Render::PostProcessing::GradingTableStatus;
using Render::PostProcessing::GradingTableWidth;
using Render::PostProcessing::LoadGradingTable;

namespace
{
class ScratchDirectory
{
public:
    ScratchDirectory()
        : m_path(std::filesystem::temp_directory_path() /
                 ("mu-grading-table-" + std::to_string(reinterpret_cast<std::uintptr_t>(this))))
    {
        std::filesystem::create_directories(m_path);
    }
    ~ScratchDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(m_path, error);
    }
    ScratchDirectory(const ScratchDirectory&) = delete;
    ScratchDirectory& operator=(const ScratchDirectory&) = delete;

    [[nodiscard]] std::filesystem::path File(const char* name) const
    {
        return m_path / name;
    }

private:
    std::filesystem::path m_path;
};

// The identity table: every entry holds its own colour.
std::uint8_t IdentityChannel(std::uint32_t entry)
{
    return static_cast<std::uint8_t>(entry * 255u / (GradingTableSize - 1u));
}

void WritePng(const std::filesystem::path& path, int width, int height)
{
    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    REQUIRE(surface != nullptr);
    auto* pixels = static_cast<std::uint8_t*>(surface->pixels);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            std::uint8_t* texel = pixels + y * surface->pitch + x * GradingTableBytesPerTexel;
            const auto column = static_cast<std::uint32_t>(x);
            texel[0] = IdentityChannel(column % GradingTableSize);
            texel[1] = IdentityChannel(static_cast<std::uint32_t>(y) % GradingTableSize);
            texel[2] = IdentityChannel(column / GradingTableSize % GradingTableSize);
            texel[3] = 255u;
        }
    }
    const bool saved = SDL_SavePNG(surface, path.string().c_str());
    SDL_DestroySurface(surface);
    REQUIRE(saved);
}

const std::uint8_t* TexelAt(const GradingTableLoad& table, std::uint32_t x, std::uint32_t y)
{
    return table.rgba.data() + (static_cast<std::size_t>(y) * GradingTableWidth + x) * GradingTableBytesPerTexel;
}
} // namespace

TEST_CASE("a 1024x32 PNG loads as a grading table [render][post_processing]")
{
    const ScratchDirectory directory;
    const auto path = directory.File("World9.png");
    WritePng(path, static_cast<int>(GradingTableWidth), static_cast<int>(GradingTableHeight));

    const GradingTableLoad table = LoadGradingTable(path);

    REQUIRE(table.status == GradingTableStatus::Loaded);
    REQUIRE(table.rgba.size() == std::size_t{GradingTableWidth} * GradingTableHeight * GradingTableBytesPerTexel);
    // Slice 31 (blue 255), red entry 0, green entry 31.
    const std::uint8_t* texel = TexelAt(table, 31u * GradingTableSize, GradingTableSize - 1u);
    CHECK(texel[0] == 0u);
    CHECK(texel[1] == 255u);
    CHECK(texel[2] == 255u);
    CHECK(texel[3] == 255u);
    // Slice 0, red entry 31, green entry 0.
    texel = TexelAt(table, GradingTableSize - 1u, 0u);
    CHECK(texel[0] == 255u);
    CHECK(texel[1] == 0u);
    CHECK(texel[2] == 0u);
}

TEST_CASE("a PNG of the wrong size is an invalid grading table [render][post_processing]")
{
    const ScratchDirectory directory;
    const auto path = directory.File("World9.png");
    WritePng(path, 512, 32);

    const GradingTableLoad table = LoadGradingTable(path);

    CHECK(table.status == GradingTableStatus::Invalid);
    CHECK(table.rgba.empty());
    CHECK(table.error.find("512x32") != std::string::npos);
}

TEST_CASE("a file that is not a PNG is an invalid grading table [render][post_processing]")
{
    const ScratchDirectory directory;
    const auto path = directory.File("World9.png");
    std::ofstream(path) << "not a png";

    const GradingTableLoad table = LoadGradingTable(path);

    CHECK(table.status == GradingTableStatus::Invalid);
    CHECK(table.rgba.empty());
    CHECK_FALSE(table.error.empty());
}

TEST_CASE("a missing file means the map has no grading table [render][post_processing]")
{
    const ScratchDirectory directory;

    const GradingTableLoad table = LoadGradingTable(directory.File("World9.png"));

    CHECK(table.status == GradingTableStatus::Missing);
    CHECK(table.rgba.empty());
    CHECK(table.error.empty());
}

TEST_CASE("a map's table is named after its number plus one [render][post_processing]")
{
    CHECK(Render::PostProcessing::GetGradingTablePath(0) == std::filesystem::path("Data/Grading/World1.png"));
    CHECK(Render::PostProcessing::GetGradingTablePath(32) == std::filesystem::path("Data/Grading/World33.png"));
}
