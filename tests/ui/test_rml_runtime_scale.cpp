#include <doctest.h>

#include "stdafx.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core.h>

#include <map>

namespace
{
struct NullRenderer : Rml::RenderInterface
{
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override { return 1; }
    void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override {}
    void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}
    Rml::TextureHandle LoadTexture(Rml::Vector2i&, const Rml::String&) override { return 0; }
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override { return 1; }
    void ReleaseTexture(Rml::TextureHandle) override {}
    void EnableScissorRegion(bool) override {}
    void SetScissorRegion(Rml::Rectanglei) override {}
};

class Contexts
{
public:
    Contexts()
    {
        Rml::SetRenderInterface(&m_Renderer);
        REQUIRE(Rml::Initialise());
        for (const char* name : {"main", "background", "dialog_background"})
            all.push_back(Rml::CreateContext(name, {1920, 1080}));
    }
    ~Contexts()
    {
        Rml::Shutdown();
        Rml::SetRenderInterface(nullptr);
        UI::Scaling::SetWindowContentScale(1.f);
    }

    std::vector<Rml::Context*> all;

private:
    NullRenderer m_Renderer;
};

// The runtime's own dp ratio: the shared fit, capped where a dialog stops growing.
RmlUiRuntimeHooks FitHooks(std::map<Rml::Context*, int>& scaled)
{
    RmlUiRuntimeHooks hooks;
    hooks.dpRatio = [](int width, int height)
    { return UI::Scaling::ViewportFitScale(width, height, UI::Scaling::MaximumPanelScale); };
    hooks.afterScale = [&scaled](Rml::Context* context) { ++scaled[context]; };
    return hooks;
}
} // namespace

TEST_CASE("a content-scale change reaches every RmlUi context at its size [ui][scaling][rmlui]")
{
    Contexts contexts;
    std::map<Rml::Context*, int> scaled;
    const RmlUiRuntimeHooks hooks = FitHooks(scaled);

    UI::Scaling::SetWindowContentScale(1.f);
    RmlUiRuntime::ApplyScale(contexts.all, hooks);
    for (Rml::Context* context : contexts.all)
        CHECK(context->GetDensityIndependentPixelRatio() == doctest::Approx(2.f));

    // 1920x1080 fits 2.25x the reference size; a content scale of 1.25 lifts the 2x cap past it.
    UI::Scaling::SetWindowContentScale(1.25f);
    scaled.clear();
    RmlUiRuntime::ApplyScale(contexts.all, hooks);
    for (Rml::Context* context : contexts.all)
    {
        CHECK(context->GetDensityIndependentPixelRatio() == doctest::Approx(2.25f));
        CHECK(context->GetDimensions() == Rml::Vector2i(1920, 1080));
        CHECK(scaled[context] == 1);
    }
}

TEST_CASE("a context not yet created is skipped [ui][scaling][rmlui]")
{
    Contexts contexts;
    std::map<Rml::Context*, int> scaled;
    Rml::Context* const partial[] = {contexts.all[0], nullptr};
    RmlUiRuntime::ApplyScale(partial, FitHooks(scaled));
    CHECK(scaled.size() == 1);
    CHECK(scaled[contexts.all[0]] == 1);
}

TEST_CASE("the reference size does not grow with the content scale [ui][scaling]")
{
    UI::Scaling::SetWindowContentScale(1.25f);
    CHECK(UI::Scaling::ViewportFitScale(640, 480, UI::Scaling::MaximumPanelScale) == doctest::Approx(1.f));
    UI::Scaling::SetWindowContentScale(1.f);
    CHECK(UI::Scaling::ViewportFitScale(640, 480, UI::Scaling::MaximumPanelScale) == doctest::Approx(1.f));
}
