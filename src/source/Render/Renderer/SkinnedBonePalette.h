#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <span>

// How much of a bone palette a skinned draw uploads. The skinning shaders range-check every bone
// index against the uploaded count, so a palette cut to the bones the vertices index draws the same.
namespace Render
{

// One past the highest bone the shader uses; 0 when the vertices index none.
template <typename Vertex> [[nodiscard]] std::uint32_t BonesIndexed(std::span<const Vertex> vertices)
{
    std::int32_t highest = -1;
    for (const Vertex& vertex : vertices)
    {
        // Match the shader: the first weight selects weighted skinning, not the legacy BMD fields.
        if constexpr (requires { vertex.weights; })
        {
            if (vertex.weights[0] != 0u)
            {
                for (std::size_t influence = 0; influence < std::size(vertex.weights); ++influence)
                {
                    if (vertex.weights[influence] > 0u)
                    {
                        highest = std::max(highest, static_cast<std::int32_t>(vertex.joints[influence]));
                    }
                }
                continue;
            }
        }
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
