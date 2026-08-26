#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace Render::Topology
{

enum class QuadSpace
{
    Screen,
    World,
};

[[nodiscard]] constexpr bool Uses3DPipeline(QuadSpace space)
{
    return space == QuadSpace::World;
}

[[nodiscard]] constexpr bool IsValidQuadVertexCount(std::size_t vertexCount)
{
    return vertexCount % 4 == 0;
}

[[nodiscard]] constexpr bool CanMergeTriangleDraws(std::uint32_t previousVertexOffset,
                                                   std::uint32_t previousVertexCount,
                                                   std::uint32_t nextVertexOffset,
                                                   std::uint32_t vertexStride)
{
    if (previousVertexCount == 0 || vertexStride == 0)
    {
        return false;
    }

    const std::uint64_t previousEnd = static_cast<std::uint64_t>(previousVertexOffset) +
                                      static_cast<std::uint64_t>(previousVertexCount) * vertexStride;
    return previousEnd == nextVertexOffset;
}

[[nodiscard]] constexpr bool CanMergeQuadDraws(std::uint32_t previousVertexOffset,
                                               std::uint32_t previousIndexCount,
                                               std::uint32_t nextVertexOffset,
                                               std::uint32_t nextIndexCount,
                                               std::uint32_t vertexStride,
                                               std::uint32_t maxQuads)
{
    constexpr std::uint32_t verticesPerQuad = 4;
    constexpr std::uint32_t indicesPerQuad = 6;

    if (previousIndexCount == 0 || nextIndexCount == 0 || vertexStride == 0 ||
        previousIndexCount % indicesPerQuad != 0 || nextIndexCount % indicesPerQuad != 0)
    {
        return false;
    }

    const std::uint64_t mergedIndexCount = static_cast<std::uint64_t>(previousIndexCount) + nextIndexCount;
    if (mergedIndexCount > static_cast<std::uint64_t>(maxQuads) * indicesPerQuad)
    {
        return false;
    }

    const std::uint64_t previousVertexCount =
        static_cast<std::uint64_t>(previousIndexCount / indicesPerQuad) * verticesPerQuad;
    const std::uint64_t previousEnd =
        static_cast<std::uint64_t>(previousVertexOffset) + previousVertexCount * vertexStride;
    return previousEnd == nextVertexOffset;
}

inline void FillQuadIndices(std::span<std::uint16_t> indices)
{
    for (std::size_t quad = 0; quad < indices.size() / 6; ++quad)
    {
        const auto base = static_cast<std::uint16_t>(quad * 4);
        const std::size_t output = quad * 6;
        indices[output + 0] = base + 0;
        indices[output + 1] = base + 1;
        indices[output + 2] = base + 2;
        indices[output + 3] = base + 0;
        indices[output + 4] = base + 2;
        indices[output + 5] = base + 3;
    }
}

} // namespace Render::Topology
