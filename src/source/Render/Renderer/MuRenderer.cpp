#include "Render/Renderer/MuRenderer.h"

namespace mu
{
namespace
{
class NoopRenderer final : public IMuRenderer
{
public:
    void RenderQuad2D(std::span<const Vertex2D>, std::uint32_t) override {}
    void RenderTriangles(std::span<const Vertex3D>, std::uint32_t) override {}
    void RenderQuadStrip(std::span<const Vertex3D>, std::uint32_t) override {}
    void SetBlendMode(BlendMode) override {}
    void DisableBlend() override {}
    void SetDepthTest(bool) override {}
    void SetFog(const FogParams&) override {}
    void BeginScene(int, int, int, int) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}
    void RenderLines(std::span<const Vertex3D>, std::uint32_t) override {}
};
} // namespace

IMuRenderer& GetRenderer()
{
    static NoopRenderer renderer;
    return renderer;
}

} // namespace mu
