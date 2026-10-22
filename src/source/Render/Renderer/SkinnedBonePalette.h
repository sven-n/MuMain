#pragma once

#include <algorithm>
#include <cstdint>
#include <span>

// How much of a bone palette a skinned draw uploads. The skinning shaders range-check every bone
// index against the uploaded count, so a palette cut to the bones the vertices index draws the same.
namespace Render
{

// One past the highest bone the vertices index (position or normal); 0 when they index none.
template <typename Vertex> [[nodiscard]] std::uint32_t BonesIndexed(std::span<const Vertex> vertices)
{
    std::int32_t highest = -1;
    for (const Vertex& vertex : vertices)
    {
        highest = std::max({highest, vertex.positionBoneIndex, vertex.normalBoneIndex});
    }
    return static_cast<std::uint32_t>(highest + 1);
}

// The bones of a `paletteBones` palette to upload: those the draw indexes, or all of them when that
// is unknown (0).
[[nodiscard]] constexpr std::uint32_t PaletteBonesToUpload(std::uint32_t paletteBones, std::uint32_t bonesIndexed)
{
    return bonesIndexed == 0u ? paletteBones : std::min(bonesIndexed, paletteBones);
}

} // namespace Render
