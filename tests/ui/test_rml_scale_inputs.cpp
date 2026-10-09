#include <doctest.h>

#include "stdafx.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlScaleInputs.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core.h>

#include <string>

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

// base.rcss's reference screens, as both themes declare them.
constexpr const char* kStyle = R"(
body { width: 100%; height: 100%; --root-scale: 1; }
.stage, .hud-board { position: absolute; left: 50%; width: 640px; height: 480px; margin-left: -320px; }
.stage { top: 50%; margin-top: -240px; transform-origin: center; transform: scale(var(--ui-scale));
         --root-scale: var(--ui-scale); }
.hud-board { bottom: 0; transform-origin: center bottom; transform: scale(var(--hud-scale));
             --root-scale: var(--hud-scale); }
.probe { position: absolute; left: 0; top: 0; width: calc(10px * var(--root-scale)); height: 10px; }
)";

class Fixture
{
public:
    explicit Fixture(Rml::Vector2i size)
    {
        Rml::SetRenderInterface(&m_Renderer);
        REQUIRE(Rml::Initialise());
        context = Rml::CreateContext("scale-inputs", size);
        REQUIRE(context != nullptr);
        UI::RmlBridge::ApplyScaleInputs(context);
    }

    ~Fixture()
    {
        Rml::Shutdown();
        Rml::SetRenderInterface(nullptr);
    }

    Rml::ElementDocument* Load(const char* screenClass)
    {
        const std::string markup = std::string("<rml><head><style>") + kStyle +
                                   R"(</style></head><body><div id="screen" class=")" + screenClass +
                                   R"("><div id="probe" class="probe"/></div></body></rml>)";
        Rml::ElementDocument* document = context->LoadDocumentFromMemory(markup);
        REQUIRE(document != nullptr);
        document->Show();
        context->Update();
        context->Render();
        return document;
    }

    Rml::Context* context = nullptr;

private:
    NullRenderer m_Renderer;
};

void Drawn(Rml::ElementDocument* document, Rml::Vector2f& offset, Rml::Vector2f& size)
{
    REQUIRE(UI::RmlBridge::DrawnContentBox(*document->GetElementById("screen"), offset, size));
}
} // namespace

TEST_CASE("a stage centres the reference screen at the UI scale [ui][scale-inputs]")
{
    Fixture fixture({1280, 720});
    Rml::ElementDocument* document = fixture.Load("stage");
    const auto panel = UI::Scaling::PanelTransform(1280, 720);

    Rml::Vector2f offset, size;
    Drawn(document, offset, size);
    CHECK(size.x == doctest::Approx(640.f * panel.scaleX));
    CHECK(offset.x == doctest::Approx(panel.offsetX));
    CHECK(offset.y == doctest::Approx(panel.offsetY));
    // The screen's --root-scale is its own scale, for the counter-scaled layers in it.
    CHECK(document->GetElementById("probe")->GetBox().GetSize().x == doctest::Approx(10.f * panel.scaleX));
}

TEST_CASE("a HUD board stands on the window's bottom at the HUD's scale [ui][scale-inputs]")
{
    Fixture fixture({1920, 1080});
    Rml::ElementDocument* document = fixture.Load("hud-board");
    const auto board = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::HudBoard, 1920, 1080);

    Rml::Vector2f offset, size;
    Drawn(document, offset, size);
    CHECK(size.x == doctest::Approx(640.f * board.scaleX));
    CHECK(offset.x == doctest::Approx(board.offsetX));
    CHECK(offset.y == doctest::Approx(board.offsetY));
}

TEST_CASE("the scale inputs follow a resized context [ui][scale-inputs]")
{
    Fixture fixture({1024, 768});
    Rml::ElementDocument* document = fixture.Load("stage");
    fixture.context->SetDimensions({1920, 1080});
    UI::RmlBridge::ApplyScaleInputs(fixture.context);
    fixture.context->Update();
    fixture.context->Render();

    const auto panel = UI::Scaling::PanelTransform(1920, 1080);
    Rml::Vector2f offset, size;
    Drawn(document, offset, size);
    CHECK(size.x == doctest::Approx(640.f * panel.scaleX));
    CHECK(offset.x == doctest::Approx(panel.offsetX));
}

TEST_CASE("a theme centres a box between the uncovered world's edges [ui][scale-inputs]")
{
    Fixture fixture({1920, 1080});
    Rml::Element* root = fixture.context->GetRootElement();
    root->SetProperty("--world-left", "400px");
    root->SetProperty("--world-right", "1500px");
    Rml::ElementDocument* document = fixture.context->LoadDocumentFromMemory(R"(<rml><head><style>
body { width: 100%; height: 100%; }
#strip { position: absolute; top: 0; height: 10px; width: calc(200px * var(--hud-scale));
         left: calc((var(--world-left) + var(--world-right)) / 2 - 100px * var(--hud-scale)); }
</style></head><body><div id="strip"/></body></rml>)");
    REQUIRE(document != nullptr);
    document->Show();
    fixture.context->Update();

    const float hud = UI::Scaling::BottomHudScale(1920, 1080);
    Rml::Element* strip = document->GetElementById("strip");
    CHECK(strip->GetBox().GetSize().x == doctest::Approx(200.f * hud));
    CHECK(strip->GetAbsoluteLeft() == doctest::Approx(950.f - 100.f * hud));

    root->SetProperty("--world-right", "1920px");
    fixture.context->Update();
    CHECK(strip->GetAbsoluteLeft() == doctest::Approx(1160.f - 100.f * hud));
}
