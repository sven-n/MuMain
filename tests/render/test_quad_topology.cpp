#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

#include <doctest.h>

#include "Render/Renderer/DrawCommandHistory.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/QuadTopology.h"

static_assert(requires(mu::IMuRenderer& renderer, std::span<const mu::Vertex3D> vertices)
{
    renderer.RenderQuad3D(vertices, 0u);
});

static_assert(requires(mu::RendererStats stats)
{
    stats.pipelineBinds;
    stats.samplerBinds;
    stats.vertexUniformPushes;
    stats.fragmentUniformPushes;
    stats.merged2DDrawCalls;
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

TEST_CASE("adjacent triangle draws require contiguous vertices for their own stride [render][quad]")
{
    constexpr std::uint32_t firstOffset = 64;
    constexpr std::uint32_t firstVertexCount = 6;
    constexpr auto stride2D = static_cast<std::uint32_t>(sizeof(mu::Vertex2D));
    constexpr auto stride3D = static_cast<std::uint32_t>(sizeof(mu::Vertex3D));

    CHECK(Render::Topology::CanMergeTriangleDraws(
        firstOffset, firstVertexCount, firstOffset + firstVertexCount * stride2D, stride2D));
    CHECK(Render::Topology::CanMergeTriangleDraws(
        firstOffset, firstVertexCount, firstOffset + firstVertexCount * stride3D, stride3D));
    CHECK_FALSE(Render::Topology::CanMergeTriangleDraws(
        firstOffset, firstVertexCount, firstOffset + firstVertexCount * stride2D + 1, stride2D));
    CHECK_FALSE(Render::Topology::CanMergeTriangleDraws(firstOffset, firstVertexCount, firstOffset, 0));
}

TEST_CASE("screen-space quads preserve the static index ceiling [render][quad]")
{
    constexpr auto stride = static_cast<std::uint32_t>(sizeof(mu::Vertex2D));
    constexpr std::uint32_t firstOffset = 128;
    constexpr std::uint32_t secondOffset = firstOffset + 4 * stride;

    CHECK(Render::Topology::CanMergeQuadDraws(firstOffset, 6, secondOffset, 6, stride, 4096));
    CHECK_FALSE(Render::Topology::CanMergeQuadDraws(firstOffset, 4096 * 6, secondOffset, 6, stride, 4096));
}

TEST_CASE("2D and 3D command families retain independent history [render][quad]")
{
    Render::DrawCommandHistory history;
    history.fill(99);

    Render::PreviousDrawCommand(history, Render::DrawCommandFamily::TextTriangles2D) = 7;
    Render::PreviousDrawCommand(history, Render::DrawCommandFamily::ScreenQuads2D) = 8;

    CHECK(Render::PreviousDrawCommand(history, Render::DrawCommandFamily::Triangles3D) == 99);
    CHECK(Render::PreviousDrawCommand(history, Render::DrawCommandFamily::Quads3D) == 99);
    CHECK(Render::PreviousDrawCommand(history, Render::DrawCommandFamily::TextTriangles2D) == 7);
    CHECK(Render::PreviousDrawCommand(history, Render::DrawCommandFamily::ScreenQuads2D) == 8);
}

namespace
{
// A backend without index buffers: keeps IMuRenderer's indexed default and records what reaches it.
class CornerRecordingRenderer final : public mu::IMuRenderer
{
public:
    std::vector<mu::Vertex3D> triangles;

    void RenderQuad2D(std::span<const mu::Vertex2D>, std::uint32_t) override {}
    void RenderTriangles(std::span<const mu::Vertex3D> vertices, std::uint32_t) override
    {
        triangles.assign(vertices.begin(), vertices.end());
    }
    void RenderQuad3D(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void RenderQuadStrip(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void SetBlendMode(mu::BlendMode) override {}
    void DisableBlend() override {}
    void SetDepthTest(bool) override {}
    void SetFog(const mu::FogParams&) override {}
    void BeginScene(int, int, int, int) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}
    void RenderLines(std::span<const mu::Vertex3D>, std::uint32_t) override {}
};
} // namespace

TEST_CASE("indexed triangles expand to their corners in index order [render][indexed]")
{
    std::array<mu::Vertex3D, 4> vertices{};
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        vertices[i].x = static_cast<float>(i);
        vertices[i].color = 0xFF000000u | static_cast<std::uint32_t>(i);
    }
    const std::array<std::uint16_t, 6> indices{2, 0, 1, 1, 3, 2};
    std::vector<mu::Vertex3D> corners;
    mu::ExpandIndexedCorners<mu::Vertex3D>(vertices, indices, corners);
    REQUIRE(corners.size() == indices.size());
    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        CHECK(std::memcmp(&corners[i], &vertices[indices[i]], sizeof(mu::Vertex3D)) == 0);
    }
}

TEST_CASE("a backend without index buffers draws indexed triangles as their corners [render][indexed]")
{
    CornerRecordingRenderer renderer;
    std::array<mu::Vertex3D, 3> vertices{};
    for (std::size_t i = 0; i < 3; ++i)
    {
        vertices[i].y = static_cast<float>(i) + 0.5f;
    }
    const std::array<std::uint16_t, 6> indices{0, 1, 2, 2, 1, 0};

    renderer.RenderIndexedTriangles(vertices, indices, 0u);
    REQUIRE(renderer.triangles.size() == 6);
    CHECK(renderer.triangles[3].y == vertices[2].y);
    CHECK(renderer.triangles[5].y == vertices[0].y);
}
