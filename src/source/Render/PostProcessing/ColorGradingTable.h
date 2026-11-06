#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

// Per-map colour grading lookup tables: `Data/Grading/World<N>.png`, N being the
// map number plus one, as in most maps' `Data/World<N>` data folder. A table is a 32x32x32 colour cube stored as
// a 1024x32 strip of 32 slices of 32x32: red runs along x inside a slice, green
// along y, blue picks the slice. The identity table maps every colour to itself.
namespace Render::PostProcessing
{

inline constexpr std::uint32_t GradingTableSize = 32u;
inline constexpr std::uint32_t GradingTableWidth = GradingTableSize * GradingTableSize;
inline constexpr std::uint32_t GradingTableHeight = GradingTableSize;
inline constexpr std::uint32_t GradingTableBytesPerTexel = 4u;

enum class GradingTableStatus : std::uint8_t
{
    Loaded,
    Missing, // the map has no table: it is not graded
    Invalid, // the file exists but cannot be used: the map is not graded
};

struct GradingTableLoad
{
    GradingTableStatus status = GradingTableStatus::Missing;
    // GradingTableWidth x GradingTableHeight RGBA8 texels, rows top to bottom; empty unless Loaded.
    std::vector<std::uint8_t> rgba;
    // Why an invalid table was rejected.
    std::string error;
};

// The table file of map `mapNumber` (0 is Lorencia).
[[nodiscard]] std::filesystem::path GetGradingTablePath(int mapNumber);

// Reads a table from a PNG file.
[[nodiscard]] GradingTableLoad LoadGradingTable(const std::filesystem::path& path);

} // namespace Render::PostProcessing
