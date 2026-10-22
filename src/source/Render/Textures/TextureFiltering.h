#pragma once

#include <cstdint>
#include <string_view>

namespace Render::Textures
{

// Whether a texture loaded from `path` is drawn in the world (terrain, map
// objects, characters, monsters, items, effects) and may therefore get a mip
// chain and an Enhanced sampler. `path` is relative to the client directory,
// e.g. "Data\\World1\\TileGrass01.jpg", with either separator and in any case.
// Only the allowlisted world folders under Data qualify; everything else
// (Interface, fonts, cursors, Logo, unknown folders) is screen-space, so an
// unknown folder can miss the enhancement but never change the UI.
[[nodiscard]] bool IsWorldTexturePath(std::wstring_view path);

// Highest anisotropy every SDL GPU backend accepts: the D3D12 and Metal
// maximum, and the minimum Vulkan guarantees once samplerAnisotropy is
// enabled (SDL only selects devices that have it). SDL exposes no query.
inline constexpr std::uint32_t MaxSupportedAnisotropy = 16;

struct AnisotropyLevel
{
    std::uint32_t value = 1;
    // The configured level was above MaxSupportedAnisotropy.
    bool clamped = false;
};

// The anisotropy level a configured value maps to: at least 1 (no
// anisotropy), at most MaxSupportedAnisotropy.
[[nodiscard]] AnisotropyLevel ClampAnisotropy(int configured);

} // namespace Render::Textures
