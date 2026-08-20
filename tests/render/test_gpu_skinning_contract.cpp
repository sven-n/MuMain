#include <doctest.h>

#include "Render/Renderer/MuRenderer.h"

namespace
{

class SkinningRendererMock final : public mu::IMuRenderer
{
public:
    void RenderQuad2D(std::span<const mu::Vertex2D>, std::uint32_t) override {}
    void RenderTriangles(std::span<const mu::Vertex3D>, std::uint32_t) override {}
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

    bool RenderSkinnedTriangles(std::span<const mu::SkinnedVertex3D> vertices, std::uint32_t textureId,
                                const mu::SkinningParameters& parameters) override
    {
        called = true;
        vertexCount = vertices.size();
        capturedTextureId = textureId;
        boneFloatCount = parameters.boneMatrices.size();
        capturedVersion = parameters.paletteVersion;
        return true;
    }

    bool called = false;
    std::size_t vertexCount = 0;
    std::uint32_t capturedTextureId = 0;
    std::size_t boneFloatCount = 0;
    std::uint32_t capturedVersion = 0;
};

} // namespace

TEST_CASE("GPU skinning draw carries rest vertices and bone palette")
{
    const mu::SkinnedVertex3D vertices[3] = {};
    const float bones[24] = {};
    const mu::SkinningParameters parameters{
        .boneMatrices = bones,
        .paletteVersion = 17,
    };
    SkinningRendererMock renderer;

    const bool accepted = renderer.RenderSkinnedTriangles(vertices, 42, parameters);

    CHECK(accepted);
    CHECK(renderer.called);
    CHECK(renderer.vertexCount == 3);
    CHECK(renderer.capturedTextureId == 42);
    CHECK(renderer.boneFloatCount == 24);
    CHECK(renderer.capturedVersion == 17);
}
