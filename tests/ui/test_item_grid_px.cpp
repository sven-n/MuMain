#include <doctest.h>

#include "stdafx.h"
#include "UI/Inventory/InventoryCtrl.h"

#include <RmlUi/Core.h>

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
        context = Rml::CreateContext("item-grid-px", {1920, 1080});
        REQUIRE(context != nullptr);
    }

    ~Fixture()
    {
        Rml::Shutdown();
        Rml::SetRenderInterface(nullptr);
    }

    // A panel placed at (300, 100) at `scale`, its 8x4 grid at (16, 90) in reference px, 20 px
    // cells, as item_grid.rcss lays them out.
    Rml::ElementDocument* Load(float scale)
    {
        const std::string markup = R"(<rml><head><style>
body { width: 100%; height: 100%; }
#panel { position: absolute; left: 300px; top: 100px; width: 190px; height: 429px; transform-origin: left top;
         transform: scale()" + std::to_string(scale) + R"(); }
#grid { position: absolute; left: 16px; top: 90px; width: 160px; height: 80px; }
.cells { display: flex; flex-wrap: wrap; width: 100%; height: 100%; }
.item-cell { position: relative; flex: 0 0 20px; width: 20px; height: 20px; }
</style></head><body><div id="panel"><div id="grid"><div class="cells">
<div class="item-cell"/><div class="item-cell"/><div class="item-cell"/></div></div></div></body></rml>)";
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
} // namespace

TEST_CASE("a pixel grid takes its origin and pitch from where the theme draws it [ui][item-grid]")
{
    Fixture fixture;
    Rml::ElementDocument* document = fixture.Load(1.5f);

    UI::Items::GridGeometry geometry;
    REQUIRE(UI::Items::DrawnGridGeometry(document->GetElementById("grid"), 8, 4, 20.f, 20.f, geometry));
    CHECK(geometry.Left() == doctest::Approx(300.f + 16.f * 1.5f));
    CHECK(geometry.Top() == doctest::Approx(100.f + 90.f * 1.5f));
    CHECK(geometry.PitchX() == doctest::Approx(30.f));
    CHECK(geometry.PitchY() == doctest::Approx(30.f));

    // The pointer in window pixels lands on the cell drawn under it.
    int column = -1;
    int row = -1;
    REQUIRE(geometry.CellAt(324.f + 2 * 30.f + 5.f, 235.f + 3 * 30.f + 29.f, column, row));
    CHECK(column == 2);
    CHECK(row == 3);
    CHECK_FALSE(geometry.Contains(324.f + 8 * 30.f + 1.f, 240.f));
}

TEST_CASE("a grid not laid out yet gives no pixel geometry [ui][item-grid]")
{
    Fixture fixture;
    fixture.Load(1.f);
    UI::Items::GridGeometry geometry;
    CHECK_FALSE(UI::Items::DrawnGridGeometry(nullptr, 8, 4, 20.f, 20.f, geometry));
}
