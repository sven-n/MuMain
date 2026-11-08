#include <doctest.h>

// Exercise the real retirement storage without a GPU or a production test hook.
#include "UI/RmlBridge/RmlRenderTarget.cpp"

namespace UI::RmlBridge
{
namespace
{
// Mirrors a global window with a by-value preview target. Its final retirement must
// precede destruction of the storage, even when rendering first uses it in main().
struct GlobalPreview
{
    RenderTarget target{nullptr};

    ~GlobalPreview()
    {
        Retire(2, "rtt:global-preview-lifetime-regression-final");
    }
};

GlobalPreview preview;

// Hands out render targets and counts the ones released through it.
class RenderTargetRecorder final : public mu::IMuRenderer
{
public:
    void RenderQuad2D(std::span<const mu::Vertex2D>, std::uint32_t) override {}
    void RenderTriangles(std::span<const mu::Vertex3D>, std::uint32_t) override {}
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

    std::uint32_t CreateRenderTarget(std::uint32_t, std::uint32_t) override
    {
        return ++created;
    }
    void ReleaseRenderTarget(std::uint32_t) override
    {
        ++released;
    }

    static constexpr std::uint32_t FirstTextureId = 1000;
    std::uint32_t created = FirstTextureId;
    int released = 0;
};

// Routes GetRenderer() to the recorder for one test.
struct RendererOverride
{
    explicit RendererOverride(mu::IMuRenderer& renderer)
    {
        mu::SetRendererOverride(&renderer);
    }
    ~RendererOverride()
    {
        mu::SetRendererOverride(nullptr);
    }
    RendererOverride(const RendererOverride&) = delete;
    RendererOverride& operator=(const RendererOverride&) = delete;
};

// Removes only the fake retirement owned by a test, even when a REQUIRE exits the scope early.
struct RetirementScope
{
    std::uint32_t id;
    Rml::String source;

    ~RetirementScope()
    {
        std::erase_if(RetiredTextures(),
                      [this](const Retired& entry) { return entry.id == id && entry.source == source; });
    }
};

bool IsRetired(std::uint32_t id)
{
    const auto& retired = RetiredTextures();
    return std::any_of(retired.begin(), retired.end(), [id](const Retired& entry) { return entry.id == id; });
}
} // namespace

TEST_CASE("global preview retirement storage outlives its owner [ui][render_target]")
{
    const std::size_t before = RetiredTextures().size();
    constexpr std::uint32_t RetainedTextureId = 3;
    {
        const RetirementScope retained{RetainedTextureId, "rtt:retained-retirement-regression"};
        Retire(retained.id, retained.source);
        {
            const RetirementScope cleanup{1, "rtt:global-preview-lifetime-regression-first"};
            Retire(cleanup.id, cleanup.source);
            CHECK(RetiredTextures().size() == before + 2);
        }
        CHECK(RetiredTextures().size() == before + 1);
        CHECK(IsRetired(retained.id));
    }
    CHECK_FALSE(IsRetired(RetainedTextureId));
    CHECK(RetiredTextures().size() == before);
}

TEST_CASE("a destroyed target retires its undrawn texture instead of using the renderer [ui][render_target]")
{
    const std::uint32_t textureId = RenderTargetRecorder::FirstTextureId + 1;
    const std::size_t before = RetiredTextures().size();
    {
        RenderTargetRecorder renderer;
        RendererOverride route(renderer);
        const RetirementScope cleanup{textureId, {}};
        {
            RenderTarget target{nullptr};
            target.Resize(64, 32);
            REQUIRE(renderer.created == textureId);
        }
        // The renderer may already be gone when a global owner is destroyed at exit.
        CHECK(renderer.released == 0);
        CHECK(IsRetired(renderer.created));
    }
    CHECK_FALSE(IsRetired(textureId));
    CHECK(RetiredTextures().size() == before);
}
} // namespace UI::RmlBridge
