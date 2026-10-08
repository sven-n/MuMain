#include <doctest.h>

#include "stdafx.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlPointer.h"

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

class Fixture
{
public:
    Fixture()
    {
        Rml::SetRenderInterface(&m_Renderer);
        REQUIRE(Rml::Initialise());
        context = Rml::CreateContext("slot-placement", {1920, 1080});
        REQUIRE(context != nullptr);
    }

    ~Fixture()
    {
        Rml::Shutdown();
        Rml::SetRenderInterface(nullptr);
    }

    Rml::ElementDocument* Load(const char* panelStyle = "")
    {
        // The docked panels' shape: reference px, scaled from its top-left; a counter-scaled layer
        // as wide as the panel at any scale.
        const std::string markup = std::string(R"(<rml><head><style>
body { width: 100%; height: 100%; pointer-events: none; --root-scale: 1; }
#panel { position: absolute; width: 190px; height: 429px; transform-origin: left top; )") +
                                   panelStyle + R"( }
#panel.slot-placed { left: var(--slot-left); top: var(--slot-top); transform: scale(var(--root-scale)); }
#panel { font-size: calc(var(--text-px, 12px) / var(--root-scale)); }
#wide { position: absolute; width: calc(190px * var(--root-scale)); height: 10px; }
</style></head><body><div id="panel"><div id="wide"/></div></body></rml>)";
        Rml::ElementDocument* document = context->LoadDocumentFromMemory(markup);
        REQUIRE(document != nullptr);
        document->Show();
        Refresh();
        return document;
    }

    // Layout, then the transforms, which resolve when the context renders.
    void Refresh()
    {
        context->Update();
        context->Render();
    }

    Rml::Context* context = nullptr;

private:
    NullRenderer m_Renderer;
};

void Drawn(Rml::ElementDocument* document, const char* id, Rml::Vector2f& offset, Rml::Vector2f& size)
{
    REQUIRE(UI::RmlBridge::DrawnContentBox(*document->GetElementById(id), offset, size));
}
} // namespace

TEST_CASE("a slot placement puts the panel at the slot, at the region's scale [ui][placement]")
{
    Fixture fixture;
    Rml::ElementDocument* document = fixture.Load();

    UI::RmlBridge::SlotPlacement placement;
    CHECK(placement.Set(1300.f, 40.f, 2.f));
    CHECK_FALSE(placement.Set(1300.f, 40.f, 2.f));
    placement.Apply(document, "panel");
    fixture.Refresh();

    Rml::Vector2f offset, size;
    Drawn(document, "panel", offset, size);
    CHECK(offset.x == doctest::Approx(1300.f));
    CHECK(offset.y == doctest::Approx(40.f));
    CHECK(size.x == doctest::Approx(380.f));
    CHECK(size.y == doctest::Approx(858.f));
    CHECK(document->GetElementById("panel")->IsClassSet("slot-placed"));
    // --root-scale follows the placement: the layer's layout width grows with it.
    CHECK(document->GetElementById("wide")->GetBox().GetSize().x == doctest::Approx(380.f));

    CHECK(placement.Set(0.f, 0.f, 0.f));
    placement.Apply(document, "panel");
    fixture.Refresh();
    Drawn(document, "panel", offset, size);
    CHECK(offset.x == doctest::Approx(0.f));
    CHECK(size.x == doctest::Approx(190.f));
    CHECK_FALSE(document->GetElementById("panel")->IsClassSet("slot-placed"));
    CHECK(document->GetElementById("wide")->GetBox().GetSize().x == doctest::Approx(190.f));
}

TEST_CASE("a placed panel's font size divides the native text size by its scale [ui][placement]")
{
    Fixture fixture;
    Rml::ElementDocument* document = fixture.Load();
    Rml::Element* panel = document->GetElementById("panel");
    panel->SetProperty("--text-px", "20px");
    UI::RmlBridge::SlotPlacement placement;
    placement.Set(0.f, 0.f, 2.f);
    placement.Apply(document, "panel");
    fixture.Refresh();
    CHECK(panel->GetComputedValues().font_size() == doctest::Approx(10.f));
}

TEST_CASE("a rebuilt document gets its slot placement again [ui][placement]")
{
    Fixture fixture;
    Rml::ElementDocument* document = fixture.Load();
    UI::RmlBridge::SlotPlacement placement;
    placement.Set(200.f, 100.f, 1.5f);
    placement.Apply(document, "panel");

    // A theme switch loads a fresh document.
    document->Close();
    document = fixture.Load();
    CHECK_FALSE(document->GetElementById("panel")->IsClassSet("slot-placed"));
    placement.Sync(document, "panel");
    fixture.Refresh();

    Rml::Vector2f offset, size;
    Drawn(document, "panel", offset, size);
    CHECK(offset.x == doctest::Approx(200.f));
    CHECK(offset.y == doctest::Approx(100.f));
    CHECK(size.x == doctest::Approx(285.f));

    // Nothing to give an unplaced window.
    UI::RmlBridge::SlotPlacement unplaced;
    document->Close();
    document = fixture.Load();
    unplaced.Sync(document, "panel");
    CHECK_FALSE(document->GetElementById("panel")->IsClassSet("slot-placed"));
}

TEST_CASE("the pointer is over a document only where it takes pointer events [ui][placement]")
{
    Fixture fixture;
    Rml::ElementDocument* document = fixture.Load("pointer-events: auto;");
    UI::RmlBridge::SlotPlacement placement;
    placement.Set(1300.f, 40.f, 2.f);
    placement.Apply(document, "panel");
    fixture.Refresh();

    // Over the scaled panel, beyond its unscaled size.
    fixture.context->ProcessMouseMove(1300 + 300, 40 + 700, 0);
    CHECK(UI::RmlBridge::IsPointerOver(document));

    fixture.context->ProcessMouseMove(1200, 500, 0);
    CHECK_FALSE(UI::RmlBridge::IsPointerOver(document));

    fixture.context->ProcessMouseMove(1310, 50, 0);
    CHECK(UI::RmlBridge::IsPointerOver(document));
    document->Hide();
    fixture.Refresh();
    CHECK_FALSE(UI::RmlBridge::IsPointerOver(document));
    CHECK_FALSE(UI::RmlBridge::IsPointerOver(static_cast<Rml::ElementDocument*>(nullptr)));
}

TEST_CASE("the pointer is over an element while RmlUi hovers it or a descendant [ui][placement]")
{
    Fixture fixture;
    Rml::ElementDocument* document = fixture.Load("pointer-events: auto;");
    Rml::Element* panel = document->GetElementById("panel");
    Rml::Element* wide = document->GetElementById("wide");

    fixture.context->ProcessMouseMove(5, 5, 0);
    CHECK(UI::RmlBridge::IsPointerOver(panel));
    CHECK(UI::RmlBridge::IsPointerOver(wide));

    fixture.context->ProcessMouseMove(5, 300, 0);
    CHECK(UI::RmlBridge::IsPointerOver(panel));
    CHECK_FALSE(UI::RmlBridge::IsPointerOver(wide));

    CHECK_FALSE(UI::RmlBridge::IsPointerOver(static_cast<Rml::Element*>(nullptr)));
}

TEST_CASE("a panel that takes no pointer events never holds the pointer [ui][placement]")
{
    Fixture fixture;
    Rml::ElementDocument* document = fixture.Load();
    fixture.context->ProcessMouseMove(50, 50, 0);
    CHECK_FALSE(UI::RmlBridge::IsPointerOver(document));
}
