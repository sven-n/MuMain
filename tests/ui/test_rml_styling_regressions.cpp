#include "stdafx.h"
#include <doctest.h>

#include "RmlLayoutFixture.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/Scaling/UITransform.h"
#include "../../src/ThirdParty/RmlUi/Source/Core/DecoratorTiled.h"
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/TextShapingContext.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
Rml::DecoratorTiled::Tile AtlasTile(Rml::DecoratorTiled::TileFitMode mode)
{
    Rml::DecoratorTiled::Tile tile;
    tile.fit_mode = mode;
    tile.size = {8.f, 6.f};
    tile.tile_data_calculated = true;
    tile.tile_data.size = tile.size;
    tile.tile_data.texcoords[0] = {0.25f, 0.25f};
    tile.tile_data.texcoords[1] = {0.5f, 0.5f};
    tile.align[0] = {Rml::Style::LengthPercentage::Percentage, 0.f};
    tile.align[1] = {Rml::Style::LengthPercentage::Percentage, 0.f};
    return tile;
}

// The area the mesh's triangles cover: the surface's area when its quads neither leave gaps nor overlap.
float CoveredArea(const Rml::Mesh& mesh)
{
    float area = 0.f;
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
    {
        const Rml::Vector2f a = mesh.vertices[mesh.indices[i]].position;
        const Rml::Vector2f b = mesh.vertices[mesh.indices[i + 1]].position;
        const Rml::Vector2f c = mesh.vertices[mesh.indices[i + 2]].position;
        area += std::abs((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y)) * 0.5f;
    }
    return area;
}
}

TEST_CASE("atlas repetition clips partial tiles without sampling adjacent sprites [ui][styling]")
{
    UI::Tests::RmlLayoutFixture fixture;
    auto* document = fixture.LoadMarkup(R"(<rml><body><div id="panel"></div></body></rml>)");
    REQUIRE(document);
    auto* panel = document->GetElementById("panel");
    // Half a texel of the 8 x 6 sprite spanning 0.25..0.5 of the texture: no sample comes closer to its edge.
    const float halfTexelX = 0.5f * 0.25f / 8.f;
    const float halfTexelY = 0.5f * 0.25f / 6.f;
    for (auto mode : {Rml::DecoratorTiled::REPEAT, Rml::DecoratorTiled::REPEAT_X, Rml::DecoratorTiled::REPEAT_Y})
    {
        CAPTURE(mode);
        Rml::Mesh mesh;
        auto tile = AtlasTile(mode);
        tile.GenerateGeometry(mesh, panel->GetComputedValues(), {}, {19.f, 14.f}, {8.f, 6.f});
        const float width = mode == Rml::DecoratorTiled::REPEAT_Y ? 8.f : 19.f;
        const float height = mode == Rml::DecoratorTiled::REPEAT_X ? 6.f : 14.f;
        CHECK(CoveredArea(mesh) == doctest::Approx(width * height));
        for (const auto& vertex : mesh.vertices)
        {
            CHECK(vertex.position.x >= 0.f);
            CHECK(vertex.position.x <= 19.f);
            CHECK(vertex.position.y >= 0.f);
            CHECK(vertex.position.y <= 14.f);
            CHECK(vertex.tex_coord.x >= 0.25f + halfTexelX - 1e-5f);
            CHECK(vertex.tex_coord.x <= 0.5f - halfTexelX + 1e-5f);
            CHECK(vertex.tex_coord.y >= 0.25f + halfTexelY - 1e-5f);
            CHECK(vertex.tex_coord.y <= 0.5f - halfTexelY + 1e-5f);
        }
        CHECK(mesh.vertices.back().position.y == height);
    }
}

TEST_CASE("a sheet sprite stays half a texel inside its sprite at every edge [ui][styling]")
{
    // The cash shop's zone tab: the upper 76 x 23 sprite of a 76 x 46 sheet, drawn at 1.6x. Its bottom edge lies on
    // the next sprite, the others on the texture border.
    UI::Tests::RmlLayoutFixture fixture;
    auto* document = fixture.LoadMarkup(R"(<rml><body><div id="panel"></div></body></rml>)");
    REQUIRE(document);
    auto* panel = document->GetElementById("panel");
    Rml::DecoratorTiled::Tile tile;
    tile.fit_mode = Rml::DecoratorTiled::FILL;
    tile.size = {76.f, 23.f};
    tile.tile_data_calculated = true;
    tile.tile_data.size = tile.size;
    tile.tile_data.texcoords[0] = {0.f, 0.f};
    tile.tile_data.texcoords[1] = {1.f, 0.5f};
    Rml::Mesh mesh;
    tile.GenerateGeometry(mesh, panel->GetComputedValues(), {}, {121.6f, 36.8f}, {121.6f, 36.8f});

    // A band half a texel wide along every edge, the sprite mapped as one quad would be in between.
    CHECK(mesh.vertices.size() == 16);
    float width = 0.f;
    float height = 0.f;
    for (const auto& vertex : mesh.vertices)
    {
        width = std::max(width, vertex.position.x);
        height = std::max(height, vertex.position.y);
    }
    for (const auto& vertex : mesh.vertices)
    {
        CAPTURE(vertex.position.x);
        CAPTURE(vertex.position.y);
        CHECK(vertex.tex_coord.x == doctest::Approx(std::clamp(vertex.position.x / width, 0.5f / 76.f, 75.5f / 76.f)));
        CHECK(vertex.tex_coord.y ==
              doctest::Approx(std::clamp(vertex.position.y / height * 0.5f, 0.5f / 46.f, 22.5f / 46.f)));
    }
    document->Close();
}

TEST_CASE("fractional letter spacing accumulates consistently in measurement and drawing [ui][styling]")
{
    UI::Tests::RmlLayoutFixture fixture;
    auto* fonts = Rml::GetFontEngineInterface();
    const auto face = fonts->GetFontFaceHandle("liberation sans", Rml::Style::FontStyle::Normal,
                                              Rml::Style::FontWeight::Normal, 16);
    REQUIRE(face);
    const Rml::String text = "iiiiiiii";
    const Rml::String language = "en";
    Rml::TextShapingContext shaping{language};
    const int baseline = fonts->GetStringWidth(face, text, shaping);
    for (float spacing : {0.25f, -0.25f, 2.f})
    {
        CAPTURE(spacing);
        shaping.letter_spacing = spacing;
        const int measured = fonts->GetStringWidth(face, text, shaping);
        CHECK(measured == baseline + static_cast<int>(spacing * text.size()));
        Rml::TexturedMeshList meshes;
        const int drawn = fonts->GenerateString(fixture.context->GetRenderManager(), face, 0, text, {},
                                                Rml::ColourbPremultiplied(255, 255, 255, 255), 1.f, shaping, meshes);
        CHECK(drawn == measured);
    }
}

TEST_CASE("chat fill respects theme insets independently of modal backdrop styles [ui][styling]")
{
    UI::Tests::RmlLayoutFixture fixture;
    for (const std::string theme : {"legacy", "modern"})
    {
        CAPTURE(theme);
        auto* document = fixture.Load(theme, "chat_log");
        REQUIRE(document);
        auto* panel = document->GetElementById("panel");
        auto* fill = document->GetElementById("chat_backdrop");
        REQUIRE(panel);
        REQUIRE(fill);
        panel->SetClass("framed", true);
        panel->SetProperty("width", "281px");
        panel->SetProperty("height", "100px");
        fill->SetProperty("left", "10px");
        fill->SetProperty("right", "10px");
        fill->SetProperty("top", "5px");
        fill->SetProperty("bottom", "5px");
        fixture.Refresh();
        CHECK(fill->GetBox().GetSize(Rml::BoxArea::Border).x == doctest::Approx(261.f));
        CHECK(fill->GetBox().GetSize(Rml::BoxArea::Border).y == doctest::Approx(90.f));
        document->Close();
    }
}

TEST_CASE("the legacy HUD's menu art reaches the cash shop button [ui][styling]")
{
    // The button row starts 1dp into #buttons; art that ended where #buttons starts left the world
    // showing through that 1dp.
    UI::Tests::RmlLayoutFixture fixture;
    fixture.Configure({1024, 768}, 100);
    auto* document = fixture.Load("legacy", "main_frame");
    REQUIRE(document);
    fixture.Refresh();
    auto* art = document->GetElementById("frame_right");
    auto* button = document->GetElementById("btn_cshop");
    REQUIRE(art);
    REQUIRE(button);
    const float artRight =
        art->GetAbsoluteOffset(Rml::BoxArea::Border).x + art->GetBox().GetSize(Rml::BoxArea::Border).x;
    const float buttonLeft = button->GetAbsoluteOffset(Rml::BoxArea::Border).x;
    CHECK(artRight >= buttonLeft - 0.01f);
    document->Close();
}

TEST_CASE("the event entry level art covers its whole band in every theme [ui][styling]")
{
    // The band is an auto-height flex box that grows to its minimum height. An absolute art child
    // sized itself against the box's height before that growth and painted only a strip.
    struct Entry
    {
        const char* theme;
        const char* window;
        Rml::Vector2i viewport;
    };
    std::vector<Entry> entries;
    for (const char* theme : {"legacy", "modern", "classic"})
        for (const char* window : {"devil_square_enter", "blood_castle_enter"})
            for (Rml::Vector2i viewport : {Rml::Vector2i{1024, 768}, Rml::Vector2i{1920, 1080}})
                entries.push_back({theme, window, viewport});

    UI::Tests::RmlLayoutFixture fixture;
    for (const auto& [theme, window, viewport] : entries)
    {
        CAPTURE(theme);
        CAPTURE(window);
        CAPTURE(viewport.x);
        fixture.Configure(viewport, 100);
        auto* document = fixture.Load(theme, window);
        REQUIRE(document);
        UI::RmlBridge::SlotPlacement placement;
        placement.Set(0.f, 0.f, UI::Scaling::DockRightTransform(viewport.x, viewport.y).scaleX);
        placement.Apply(document, "panel");
        auto* band = document->QuerySelector(".entry-btn");
        REQUIRE(band);
        band->SetClass("enabled", true);
        auto* label = band->QuerySelector(".entry-label");
        REQUIRE(label);
        label->SetInnerRML("The 1 Square (15-130 level)");
        fixture.Refresh();
        const Rml::Vector2f bandSize = band->GetBox().GetSize(Rml::BoxArea::Border);
        REQUIRE(bandSize.y > label->GetBox().GetSize(Rml::BoxArea::Border).y);
        Rml::ElementList painters;
        band->QuerySelectorAll(painters, "*");
        painters.push_back(band);
        int painted = 0;
        for (auto* painter : painters)
        {
            const auto& computed = painter->GetComputedValues();
            if (!computed.has_decorator() && computed.background_color().alpha == 0)
                continue;
            ++painted;
            const Rml::Vector2f size = painter->GetBox().GetSize(Rml::BoxArea::Border);
            CHECK(size.x == doctest::Approx(bandSize.x));
            CHECK(size.y == doctest::Approx(bandSize.y));
        }
        CHECK(painted > 0);
        fixture.context->UnloadDocument(document);
        fixture.Refresh();
    }
}

namespace
{
struct DrawnRect
{
    float left = 0.f;
    float top = 0.f;
    float right = 0.f;
    float bottom = 0.f;
};

DrawnRect Drawn(Rml::Element& element)
{
    Rml::Vector2f offset, size;
    REQUIRE(UI::RmlBridge::DrawnBox(element, Rml::BoxArea::Border, offset, size));
    return {offset.x, offset.y, offset.x + size.x, offset.y + size.y};
}

Rml::Element& Required(Rml::ElementDocument& document, const char* selector)
{
    Rml::Element* element = document.QuerySelector(selector);
    CAPTURE(selector);
    REQUIRE(element != nullptr);
    return *element;
}
} // namespace

TEST_CASE("the legacy character window shows the points to spend between the summary and STR [ui][styling]")
{
    // The original drew "Point: N" beside the level (x 110); "Level: N | Resets: N" runs into it.
    // newui_item_table03(Dw) draws the summary's bottom line on its rows 7-9 of 14.
    constexpr float BottomLineEndRow = 10.f;
    constexpr float BottomEdgeRows = 14.f;
    constexpr float Tolerance = 0.5f;
    UI::Tests::RmlLayoutFixture fixture;
    // A display scale above 100% grows the text but not a small window; such windows are only listed
    // where the screen gives the panel room to grow too.
    struct Screen
    {
        Rml::Vector2i viewport;
        int percent;
        float contentScale;
    };
    const Screen screens[] = {{{800, 600}, 100, 1.f},    {{1024, 768}, 75, 1.f},    {{1024, 768}, 100, 1.f},
                              {{1280, 720}, 100, 1.f},   {{1280, 720}, 150, 1.f},   {{1920, 1080}, 100, 1.f},
                              {{1920, 1080}, 125, 1.f},  {{1920, 1080}, 150, 1.f},  {{2560, 1440}, 100, 1.f},
                              {{1920, 1080}, 100, 1.5f}, {{2560, 1440}, 100, 1.5f}, {{3840, 2160}, 100, 2.f}};
    for (const auto& [viewport, percent, contentScale] : screens)
    {
        CAPTURE(viewport.x);
        CAPTURE(viewport.y);
        CAPTURE(percent);
        CAPTURE(contentScale);
        fixture.Configure(viewport, percent, contentScale);
        auto* document = fixture.Load("legacy", "character_info");
        REQUIRE(document);
        UI::RmlBridge::SlotPlacement placement;
        placement.Set(0.f, 0.f, UI::Scaling::DockRightTransform(viewport.x, viewport.y).scaleX);
        placement.Apply(document, "panel");
        const std::string fontSize =
            std::to_string(UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, viewport.x, viewport.y)) +
            "px";
        auto& level = Required(*document, ".level-text");
        auto& points = Required(*document, ".levelup-point-text");
        level.SetProperty("font-size", fontSize);
        points.SetProperty("font-size", fontSize);
        level.SetInnerRML("Level: 400 | Resets: 65535");
        points.SetInnerRML("Point: 65535");
        fixture.Refresh();

        const DrawnRect levelBox = Drawn(level);
        const DrawnRect pointsBox = Drawn(points);
        const DrawnRect bottomEdge = Drawn(Required(*document, ".edge-bottom"));
        const DrawnRect summary = Drawn(Required(*document, ".summary-fill"));
        const DrawnRect strength = Drawn(Required(*document, ".attr-textbox-str"));
        const float bottomLine =
            bottomEdge.top + (bottomEdge.bottom - bottomEdge.top) * BottomLineEndRow / BottomEdgeRows;
        CHECK(pointsBox.top >= bottomLine - Tolerance);
        CHECK(pointsBox.bottom <= strength.top + Tolerance);
        CHECK(pointsBox.right <= summary.right + Tolerance);
        CHECK((pointsBox.top >= levelBox.bottom || pointsBox.left >= levelBox.right));

        // The rows moved down to make the gap; the Dark Lord's Command row still clears the action row.
        const DrawnRect command = Drawn(Required(*document, ".attr-textbox-cmd"));
        const DrawnRect actions = Drawn(Required(*document, ".info-btn-quest"));
        CHECK(command.bottom <= actions.top);

        fixture.context->UnloadDocument(document);
        fixture.Refresh();
    }
}

TEST_CASE("the legacy options tabs each paint their own tab, so a separator sits between every two [ui][styling]")
{
    // One 3-tab strip stretched under the whole row put its two separators across the labels.
    const auto stylesheet = (std::filesystem::path(MU_RMLUI_DIR) / "themes/legacy/option_window.rcss").generic_string();
    UI::Tests::RmlLayoutFixture fixture;
    fixture.Configure({1024, 768}, 100);
    auto* document = fixture.LoadMarkup(
        R"(<rml><head><link type="text/rcss" href=")" + stylesheet +
        R"("/></head><body><div id="row" class="option-tab-row" style="width: 404dp;">)"
        R"(<div class="option-tab-btn active-tab">Display</div><div class="option-tab-btn">Graphics</div>)"
        R"(<div class="option-tab-btn">Audio</div><div class="option-tab-btn">Gameplay</div>)"
        R"(<div class="option-tab-btn">Interface</div></div></body></rml>)");
    REQUIRE(document);
    auto* row = document->GetElementById("row");
    REQUIRE(row);
    Rml::ElementList tabs;
    row->QuerySelectorAll(tabs, ".option-tab-btn");
    REQUIRE(tabs.size() == 5);
    CHECK_FALSE(row->GetComputedValues().has_decorator());
    const float rowLeft = row->GetAbsoluteOffset(Rml::BoxArea::Content).x;
    float edge = rowLeft;
    for (auto* tab : tabs)
    {
        CHECK(tab->GetComputedValues().has_decorator());
        const float left = tab->GetAbsoluteOffset(Rml::BoxArea::Border).x;
        CHECK(left == doctest::Approx(edge));
        edge = left + tab->GetBox().GetSize(Rml::BoxArea::Border).x;
    }
    CHECK(edge == doctest::Approx(rowLeft + row->GetBox().GetSize(Rml::BoxArea::Content).x));
    document->Close();
}
