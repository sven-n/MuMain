#include "Render/PostProcessing/ColorGradingTable.h"

#include <SDL3/SDL.h>

#include <cstring>
#include <memory>
#include <system_error>

namespace Render::PostProcessing
{
namespace
{
struct SurfaceDeleter
{
    void operator()(SDL_Surface* surface) const
    {
        SDL_DestroySurface(surface);
    }
};
using SurfacePointer = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

GradingTableLoad Invalid(std::string error)
{
    return {GradingTableStatus::Invalid, {}, std::move(error)};
}

std::vector<std::uint8_t> CopyTexels(const SDL_Surface& surface)
{
    constexpr std::size_t rowBytes = std::size_t{GradingTableWidth} * GradingTableBytesPerTexel;
    std::vector<std::uint8_t> rgba(rowBytes * GradingTableHeight);
    const auto* source = static_cast<const std::uint8_t*>(surface.pixels);
    for (std::uint32_t row = 0; row < GradingTableHeight; ++row)
    {
        std::memcpy(rgba.data() + row * rowBytes, source + static_cast<std::size_t>(row) * surface.pitch, rowBytes);
    }
    return rgba;
}
} // namespace

std::filesystem::path GetGradingTablePath(int mapNumber)
{
    return std::filesystem::path("Data") / "Grading" / ("World" + std::to_string(mapNumber + 1) + ".png");
}

GradingTableLoad LoadGradingTable(const std::filesystem::path& path)
{
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error))
    {
        return {GradingTableStatus::Missing, {}, {}};
    }

    const SurfacePointer loaded{SDL_LoadPNG(path.string().c_str())};
    if (!loaded)
    {
        return Invalid(std::string("not a readable PNG: ") + SDL_GetError());
    }
    if (loaded->w != static_cast<int>(GradingTableWidth) || loaded->h != static_cast<int>(GradingTableHeight))
    {
        return Invalid("size " + std::to_string(loaded->w) + "x" + std::to_string(loaded->h) + ", expected " +
                       std::to_string(GradingTableWidth) + "x" + std::to_string(GradingTableHeight));
    }

    // RGBA32 is R, G, B, A in memory on every platform.
    const SurfacePointer converted{SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_RGBA32)};
    if (!converted || !SDL_LockSurface(converted.get()))
    {
        return Invalid(std::string("cannot convert to RGBA: ") + SDL_GetError());
    }
    std::vector<std::uint8_t> rgba = CopyTexels(*converted);
    SDL_UnlockSurface(converted.get());
    return {GradingTableStatus::Loaded, std::move(rgba), {}};
}

} // namespace Render::PostProcessing
