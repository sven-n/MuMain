#include <array>
#include <cstdint>

#include <doctest.h>

#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/QuadTopology.h"

static_assert(requires(mu::IMuRenderer& renderer, std::span<const mu::Vertex3D> vertices)
{
    renderer.RenderQuad3D(vertices, 0u);
});

TEST_CASE("independent quads require four vertices [render][quad]")
{
    CHECK(Render::Topology::IsValidQuadVertexCount(0));
    CHECK(Render::Topology::IsValidQuadVertexCount(4));
    CHECK(Render::Topology::IsValidQuadVertexCount(8));
    CHECK_FALSE(Render::Topology::IsValidQuadVertexCount(3));
    CHECK_FALSE(Render::Topology::IsValidQuadVertexCount(6));
}

TEST_CASE("independent quads use perimeter triangle indices [render][quad]")
{
    std::array<std::uint16_t, 12> indices{};

    Render::Topology::FillQuadIndices(indices);

    CHECK(indices == std::array<std::uint16_t, 12>{0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7});
}

TEST_CASE("world-space quads select the 3D pipeline [render][quad]")
{
    CHECK(Render::Topology::Uses3DPipeline(Render::Topology::QuadSpace::World));
    CHECK_FALSE(Render::Topology::Uses3DPipeline(Render::Topology::QuadSpace::Screen));
}

TEST_CASE("adjacent quad draws merge within the static index capacity [render][quad]")
{
    constexpr auto vertexStride = static_cast<std::uint32_t>(sizeof(mu::Vertex3D));
    constexpr std::uint32_t firstOffset = 64;
    constexpr std::uint32_t secondOffset = firstOffset + 4 * vertexStride;

    CHECK(Render::Topology::CanMergeQuadDraws(firstOffset, 6, secondOffset, 6, vertexStride, 4096));
    CHECK_FALSE(Render::Topology::CanMergeQuadDraws(firstOffset, 6, secondOffset + vertexStride, 6, vertexStride,
                                                    4096));
    CHECK_FALSE(Render::Topology::CanMergeQuadDraws(firstOffset, 4096 * 6, secondOffset, 6, vertexStride, 4096));
}
