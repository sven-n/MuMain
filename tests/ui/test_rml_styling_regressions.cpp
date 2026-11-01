#include "stdafx.h"
#include <doctest.h>

#include "RmlLayoutFixture.h"
#include "Data/GameData/ServerListData/LoadBar.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/Scaling/UITransform.h"
#include "../../src/ThirdParty/RmlUi/Source/Core/DecoratorTiled.h"
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/StyleSheetTypes.h>
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

// The element's decorator as the stylesheet declares it, e.g. "ninepatch(option-tab-idle, ...)".
std::string DecoratorText(Rml::Element& element)
{
    const Rml::Property* property = element.GetProperty(Rml::PropertyId::Decorator);
    if (property == nullptr || property->unit != Rml::Unit::DECORATOR)
        return {};
    const auto decorators = property->Get<Rml::DecoratorsPtr>();
    return decorators ? decorators->value : std::string{};
}

// A legacy options tab row of `tabCount` tabs with tab `active` selected, styled by the theme's sheet.
std::string OptionTabRowMarkup(int tabCount, int active)
{
    const auto stylesheet = (std::filesystem::path(MU_RMLUI_DIR) / "themes/legacy/option_window.rcss").generic_string();
    std::string tabs;
    for (int tab = 0; tab < tabCount; ++tab)
        tabs += std::string(R"(<div class="option-tab-btn)") + (tab == active ? " active-tab" : "") + R"(">Tab</div>)";
    return R"(<rml><head><link type="text/rcss" href=")" + stylesheet +
           R"("/></head><body><div id="row" class="option-tab-row" style="width: 404dp;">)" + tabs +
           "</div></body></rml>";
}

// What the element paints: its decorator and the colours that tint, edge or fill it.
std::string PaintText(Rml::Element& element)
{
    const auto& computed = element.GetComputedValues();
    return DecoratorText(element) + '|' + Rml::ToString(computed.image_color()) + '|' +
           Rml::ToString(computed.border_top_color()) + '|' + Rml::ToString(computed.background_color());
}

// An enabled band lights its art under the mouse and when pressed; legacy swaps the sprite.
void CheckEntryArtStates(UI::Tests::RmlLayoutFixture& fixture, const std::string& theme, Rml::Element& band)
{
    auto* art = band.QuerySelector(".entry-btn-art");
    REQUIRE(art);
    const std::string idle = PaintText(*art);
    band.SetPseudoClass("hover", true);
    fixture.Refresh();
    const std::string hovered = PaintText(*art);
    band.SetPseudoClass("active", true);
    fixture.Refresh();
    const std::string pressed = PaintText(*art);
    CHECK(hovered != idle);
    CHECK(pressed != idle);
    if (theme != "legacy")
        return;
    CHECK(idle.find("(entry-btn-up)") != std::string::npos);
    CHECK(hovered.find("(entry-btn-over)") != std::string::npos);
    CHECK(pressed.find("(entry-btn-down)") != std::string::npos);
}

// Lays out the first level band of an event entry window and checks that whatever paints it covers
// the whole band.
void CheckEntryArt(UI::Tests::RmlLayoutFixture& fixture, const std::string& theme, const std::string& window,
                   Rml::Vector2i viewport)
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
    CheckEntryArtStates(fixture, theme, *band);
    fixture.context->UnloadDocument(document);
    fixture.Refresh();
}
} // namespace

TEST_CASE("the event entry level art covers its whole band in every theme [ui][styling]")
{
    // The band is an auto-height flex box that grows to its minimum height, and its art an absolute
    // child (classic: a block band). RmlUi used to size such a child against the flex box's height
    // from before that growth (FlexContainer::Close kept its pre-layout box), so the art covered only
    // the band's padding: a strip above the label.
    UI::Tests::RmlLayoutFixture fixture;
    for (const char* theme : {"legacy", "modern", "classic"})
        for (const char* window : {"devil_square_enter", "blood_castle_enter"})
            for (Rml::Vector2i viewport : {Rml::Vector2i{1024, 768}, Rml::Vector2i{1920, 1080}})
                CheckEntryArt(fixture, theme, window, viewport);
}

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
        // Its transform writes base.rcss's counter-scale out; it must still match the other texts'.
        CHECK(UI::RmlBridge::DrawnScale(points) == doctest::Approx(UI::RmlBridge::DrawnScale(level)));
        CHECK((pointsBox.top >= levelBox.bottom || pointsBox.left >= levelBox.right));

        // The rows moved down to make the gap; the Dark Lord's Command row still clears the action row.
        const DrawnRect command = Drawn(Required(*document, ".attr-textbox-cmd"));
        const DrawnRect actions = Drawn(Required(*document, ".info-btn-quest"));
        CHECK(command.bottom <= actions.top);

        fixture.context->UnloadDocument(document);
        fixture.Refresh();
    }
}

TEST_CASE("every legacy attribute row sits the points' room lower than the original [ui][styling]")
{
    // Each row's top is the original's plus --points-room. A var() that failed to resolve would leave
    // a row at its original top or at 0, at some UI scales only, so the small ones are included.
    constexpr float PointsRoom = 8.f;
    constexpr size_t RowElements = 24; // 5 text boxes, labels, values and buttons; 4 line blocks
    UI::Tests::RmlLayoutFixture fixture;
    const std::pair<Rml::Vector2i, int> screens[] = {
        {{800, 600}, 100}, {{1024, 768}, 75}, {{1024, 768}, 100}, {{1920, 1080}, 100}};
    for (const auto& [viewport, percent] : screens)
    {
        CAPTURE(viewport.x);
        CAPTURE(percent);
        fixture.Configure(viewport, percent);
        auto* document = fixture.Load("legacy", "character_info");
        REQUIRE(document);
        UI::RmlBridge::SlotPlacement placement;
        placement.Set(0.f, 0.f, UI::Scaling::DockRightTransform(viewport.x, viewport.y).scaleX);
        placement.Apply(document, "panel");
        fixture.Refresh();
        Rml::ElementList rows;
        document->QuerySelectorAll(rows, ".attr-textbox, .attr-label, .attr-value, .attr-lines, .stat-btn");
        REQUIRE(rows.size() == RowElements);
        std::vector<float> tops;
        for (auto* row : rows)
            tops.push_back(row->GetAbsoluteOffset(Rml::BoxArea::Border).y);
        Required(*document, "#panel").SetProperty("--points-room", "0px");
        fixture.Refresh();
        for (size_t row = 0; row < rows.size(); ++row)
        {
            CAPTURE(rows[row]->GetClassNames());
            CHECK(tops[row] - rows[row]->GetAbsoluteOffset(Rml::BoxArea::Border).y == doctest::Approx(PointsRoom));
        }
        fixture.context->UnloadDocument(document);
        fixture.Refresh();
    }
}

TEST_CASE("the legacy options tabs each paint their own tab, so a separator sits between every two [ui][styling]")
{
    // One 3-tab strip stretched under the whole row put its two separators across the labels. Each
    // tab now paints one cell of it; the last also takes the strip's closing column, and the selected
    // tab keeps its own art wherever it is.
    constexpr int TabCount = 5;
    UI::Tests::RmlLayoutFixture fixture;
    fixture.Configure({1024, 768}, 100);
    for (int active : {0, TabCount - 1})
    {
        CAPTURE(active);
        auto* document = fixture.LoadMarkup(OptionTabRowMarkup(TabCount, active));
        REQUIRE(document);
        auto* row = document->GetElementById("row");
        REQUIRE(row);
        Rml::ElementList tabs;
        row->QuerySelectorAll(tabs, ".option-tab-btn");
        REQUIRE(tabs.size() == TabCount);
        CHECK_FALSE(row->GetComputedValues().has_decorator());
        const float rowLeft = row->GetAbsoluteOffset(Rml::BoxArea::Content).x;
        float edge = rowLeft;
        for (int tab = 0; tab < TabCount; ++tab)
        {
            CAPTURE(tab);
            const char* sprite = tab == active         ? "option-tab-active"
                                 : tab == TabCount - 1 ? "option-tab-idle-last"
                                                       : "option-tab-idle";
            CHECK(DecoratorText(*tabs[tab]).find(std::string("(") + sprite + ",") != std::string::npos);
            const float left = tabs[tab]->GetAbsoluteOffset(Rml::BoxArea::Border).x;
            CHECK(left == doctest::Approx(edge));
            edge = left + tabs[tab]->GetBox().GetSize(Rml::BoxArea::Border).x;
        }
        CHECK(edge == doctest::Approx(rowLeft + row->GetBox().GetSize(Rml::BoxArea::Content).x));
        document->Close();
    }
}

namespace
{
// A box in layout coordinates, the same for every element of one document.
struct LayoutRect
{
    float left = 0.f;
    float top = 0.f;
    float right = 0.f;
    float bottom = 0.f;

    float Width() const { return right - left; }
    float Middle() const { return (left + right) * 0.5f; }
    float Center() const { return (top + bottom) * 0.5f; }
};

LayoutRect LaidOut(Rml::Element& element, Rml::BoxArea area)
{
    const Rml::Vector2f offset = element.GetAbsoluteOffset(area);
    const Rml::Vector2f size = element.GetBox().GetSize(area);
    return {offset.x, offset.y, offset.x + size.x, offset.y + size.y};
}

float BorderWidth(Rml::Element& element)
{
    return element.GetBox().GetSize(Rml::BoxArea::Border).x;
}

Rml::Element& Child(Rml::Element& parent, const char* selector)
{
    Rml::Element* element = parent.QuerySelector(selector);
    CAPTURE(selector);
    REQUIRE(element != nullptr);
    return *element;
}

constexpr int ChannelRowCount = 20;
constexpr int LongChannelRow = 2;
constexpr const char* LongChannelName = "Valhalla-20 (Gold Non-PvP Premium Plus)";
constexpr const char* LongLoadText = "Every place on this server is taken";

// What the channel list shows (server_select.rml without its bindings): its rows' names and the load text of
// the row LongChannelRow, and the texts of its sizer row.
struct ChannelListTexts
{
    int count = 1;
    // The name of the row LongChannelRow; the others are short ("Valhalla-1").
    std::string longName;
    // The load text of the row LongChannelRow; empty while it can be joined.
    std::string load;
    std::vector<std::string> sizerNames = {"Valhalla-1"};
    std::vector<std::string> sizerLoads;
};

struct ChannelList
{
    Rml::ElementDocument* document = nullptr;
    std::vector<Rml::Element*> rows;
    Rml::Element* sizer = nullptr;
};

// A load text as the server list styles it: bold, not italic, as the fixture has no italic face and a text
// without its face is laid out with no width.
std::string LoadTextMarkup(const std::string& text, const char* classes)
{
    return std::string(R"(<div class=")") + classes + R"( bold">)" + text + "</div>";
}

ChannelList LoadChannelList(UI::Tests::RmlLayoutFixture& fixture, const char* theme, const ChannelListTexts& texts)
{
    std::string markup = UI::Tests::AssetMarkup(theme, "server_select");
    if (std::string(theme) == "classic")
    {
        // Classic builds on legacy (its theme.ini's Base): the game gives it legacy's base.rcss.
        const std::string root = std::filesystem::path(MU_RMLUI_DIR).generic_string();
        const std::string shared = "href=\"" + root + "/base.rcss\"";
        const size_t link = markup.find(shared);
        REQUIRE(link != std::string::npos);
        markup.replace(link, shared.size(), "href=\"" + root + "/themes/legacy/base.rcss\"");
    }
    ChannelList list;
    list.document = fixture.LoadMarkup(markup);
    REQUIRE(list.document);
    // The root size the game gives a "scene-window-scale" document in a theme with native text sizes
    // (RmlNativeText.h): 1rem is one original pixel. Classic builds on legacy.
    if (std::string(theme) != "modern")
    {
        const Rml::Vector2i window = fixture.context->GetDimensions();
        list.document->SetProperty(Rml::PropertyId::FontSize,
                                   Rml::Property(UI::Scaling::SceneWindowScale(window.x, window.y), Rml::Unit::PX));
    }
    list.sizer = &Required(*list.document, ".server-row-sizer");
    Rml::Element* templateRow = list.document->QuerySelector(".server-row");
    REQUIRE(templateRow);
    REQUIRE(templateRow != list.sizer);
    Rml::Element* rows = templateRow->GetParentNode();
    for (int row = 1; row < texts.count; ++row)
        rows->InsertBefore(templateRow->Clone(), list.sizer);
    for (int row = 0; row < rows->GetNumChildren(); ++row)
    {
        Rml::Element* channel = rows->GetChild(row);
        if (channel == list.sizer)
            continue;
        const bool isLong = row == LongChannelRow;
        Child(*channel, ".server-name")
            .SetInnerRML(isLong && !texts.longName.empty() ? texts.longName : "Valhalla-" + std::to_string(row + 1));
        // Every row has the bar's ten segments; a full one has its load text's line over them too.
        std::string segments;
        for (int segment = 0; segment < Data::ServerList::LoadBarSegmentCount; ++segment)
            segments += R"(<div class="server-gauge-segment"><div class="server-gauge-fill"></div></div>)";
        Child(*channel, ".server-gauge-slot").SetInnerRML(segments);
        Rml::Element& loadLine = Child(*channel, ".server-load-line");
        if (isLong && !texts.load.empty())
        {
            Rml::Element& load = Child(loadLine, ".server-load");
            load.SetInnerRML(texts.load);
            load.SetClass("bold", true);
        }
        else
        {
            channel->RemoveChild(&loadLine);
        }
        list.rows.push_back(channel);
    }
    std::string names;
    for (const std::string& name : texts.sizerNames)
        names += R"(<div class="server-sizer">)" + name + "</div>";
    std::string loads;
    for (const std::string& load : texts.sizerLoads)
        loads += LoadTextMarkup(load, "server-load server-sizer");
    Child(*list.sizer, ".server-name").SetInnerRML(names);
    Child(*list.sizer, ".server-load-sizer").SetInnerRML(loads);
    fixture.Refresh();
    return list;
}

void Unload(UI::Tests::RmlLayoutFixture& fixture, const ChannelList& list)
{
    list.document->Close();
    fixture.Refresh();
}

// The row's bar segments, as laid out.
std::vector<LayoutRect> Segments(Rml::Element& row)
{
    Rml::ElementList segments;
    row.QuerySelectorAll(segments, ".server-gauge-segment");
    std::vector<LayoutRect> rects;
    for (Rml::Element* segment : segments)
        rects.push_back(LaidOut(*segment, Rml::BoxArea::Border));
    return rects;
}

// The segments a full row's load text covers, measured as CServerSelWin::FitLoadTexts does.
Data::ServerList::CoveredSegments CoveredByLoadText(Rml::Element& row)
{
    std::vector<Data::ServerList::SegmentSpan> spans;
    for (const LayoutRect& segment : Segments(row))
        spans.push_back({segment.left, segment.Width()});
    const LayoutRect text = LaidOut(Child(row, ".server-load"), Rml::BoxArea::Border);
    return Data::ServerList::GetCoveredSegments(spans, text.left, text.right);
}

// The width of the rows of a list with these texts.
float RowWidth(UI::Tests::RmlLayoutFixture& fixture, const char* theme, const ChannelListTexts& texts)
{
    const ChannelList list = LoadChannelList(fixture, theme, texts);
    const float width = BorderWidth(*list.rows.front());
    Unload(fixture, list);
    return width;
}
} // namespace

TEST_CASE("every channel row is as wide as the longest text any channel can have, in every theme [ui][styling]")
{
    // The rows had a fixed width, so a long text ran out of its button; sized by the texts shown, they grew
    // whenever a server became full or overloaded. And a scrolling list's scrollbar takes its width from
    // content sized without it, which would squeeze the rows again.
    constexpr float Tolerance = 1.f;
    UI::Tests::RmlLayoutFixture fixture;
    for (const char* theme : {"legacy", "modern", "classic"})
    {
        for (const Rml::Vector2i viewport : {Rml::Vector2i{1024, 768}, Rml::Vector2i{1920, 1080}})
        {
            CAPTURE(std::string(theme));
            CAPTURE(viewport.x);
            fixture.Configure(viewport, 100);

            const float shortWidth = RowWidth(fixture, theme, {});

            ChannelListTexts texts;
            texts.count = ChannelRowCount;
            texts.longName = LongChannelName;
            texts.load = "Overloaded";
            texts.sizerNames = {"Valhalla-1", LongChannelName, "Midgard-1"};
            texts.sizerLoads = {"Overloaded", "(Full)"};
            const ChannelList list = LoadChannelList(fixture, theme, texts);
            REQUIRE(list.rows.size() == ChannelRowCount);
            Rml::Element& column = Required(*list.document, "#server_col");
            REQUIRE(column.GetScrollHeight() > column.GetClientHeight());
            Rml::Element& longRow = *list.rows[LongChannelRow];
            Rml::Element& openRow = *list.rows.front();
            const float width = BorderWidth(longRow);
            CHECK(width > shortWidth + Tolerance);
            for (Rml::Element* row : list.rows)
                CHECK(BorderWidth(*row) == doctest::Approx(width));

            // The name at the left of the row's text area, and it fits.
            const LayoutRect content = LaidOut(longRow, Rml::BoxArea::Content);
            Rml::Element& nameBox = Child(longRow, ".server-name");
            const LayoutRect name = LaidOut(nameBox, Rml::BoxArea::Border);
            CHECK(name.left == doctest::Approx(content.left));
            CHECK(nameBox.GetScrollWidth() <= name.Width() + Tolerance);

            // The load text centered on the bar, whose segments are where an open row has them; those it covers
            // hide, the same number on each side, at least one stays on each side and none of the others is
            // under it.
            const LayoutRect openRowBox = LaidOut(openRow, Rml::BoxArea::Border);
            const LayoutRect gauge = LaidOut(Child(openRow, ".server-gauge-slot"), Rml::BoxArea::Border);
            const LayoutRect row = LaidOut(longRow, Rml::BoxArea::Border);
            const LayoutRect line = LaidOut(Child(longRow, ".server-load-line"), Rml::BoxArea::Border);
            CHECK(line.Center() - row.top == doctest::Approx(gauge.Center() - openRowBox.top).epsilon(0.05));
            CHECK(line.bottom <= row.bottom);
            const LayoutRect load = LaidOut(Child(longRow, ".server-load"), Rml::BoxArea::Border);
            CHECK(load.Width() > Tolerance);
            CHECK(load.Middle() - row.left == doctest::Approx(gauge.Middle() - openRowBox.left).epsilon(0.01));
            const std::vector<LayoutRect> openSegments = Segments(openRow);
            const std::vector<LayoutRect> fullSegments = Segments(longRow);
            REQUIRE(fullSegments.size() == static_cast<size_t>(Data::ServerList::LoadBarSegmentCount));
            REQUIRE(openSegments.size() == fullSegments.size());
            for (size_t segment = 0; segment < fullSegments.size(); ++segment)
            {
                CAPTURE(segment);
                CHECK(fullSegments[segment].left - row.left ==
                      doctest::Approx(openSegments[segment].left - openRowBox.left));
                CHECK(fullSegments[segment].Width() == doctest::Approx(openSegments[segment].Width()));
            }
            const Data::ServerList::CoveredSegments covered = CoveredByLoadText(longRow);
            CHECK(covered.first >= 1);
            CHECK(covered.end > covered.first);
            CHECK(covered.first == Data::ServerList::LoadBarSegmentCount - covered.end);
            for (int segment = 0; segment < Data::ServerList::LoadBarSegmentCount; ++segment)
            {
                if (segment >= covered.first && segment < covered.end)
                    continue;
                CAPTURE(segment);
                const LayoutRect& shown = fullSegments[static_cast<size_t>(segment)];
                CHECK((shown.right <= load.left + 0.01f || shown.left >= load.right - 0.01f));
            }

            // The scrollbar sits beside the rows and takes no width from them.
            Rml::Element* scrollbar = nullptr;
            for (int child = 0; child < column.GetNumChildren(true); ++child)
            {
                if (column.GetChild(child)->GetTagName() == "scrollbarvertical")
                    scrollbar = column.GetChild(child);
            }
            REQUIRE(scrollbar);
            CHECK(row.right <= LaidOut(*scrollbar, Rml::BoxArea::Border).left + 0.01f);

            // The sizer row takes no room after the last row.
            const LayoutRect first = LaidOut(*list.rows.front(), Rml::BoxArea::Border);
            const LayoutRect last = LaidOut(*list.rows.back(), Rml::BoxArea::Border);
            CHECK(LaidOut(*list.sizer, Rml::BoxArea::Border).bottom <= last.bottom + 0.01f);
            CHECK(column.GetScrollHeight() <= last.bottom - first.top + Tolerance);

            // The bar spans the row below the name.
            const LayoutRect openName = LaidOut(Child(openRow, ".server-name"), Rml::BoxArea::Border);
            CHECK(gauge.left >= openRowBox.left);
            CHECK(gauge.right <= openRowBox.right);
            CHECK(gauge.Width() > (openRowBox.right - openRowBox.left) * 0.75f);
            CHECK(gauge.top >= openName.bottom);
            CHECK(gauge.bottom <= openRowBox.bottom);
            if (std::string(theme) != "classic")
                CHECK(line.top >= LaidOut(Child(longRow, ".server-name"), Rml::BoxArea::Border).bottom - 0.01f);
            if (std::string(theme) == "legacy")
            {
                // At least the bar's own height free above it, to the name, and below it, to the frame's edge.
                const float barHeight = gauge.bottom - gauge.top;
                CHECK(gauge.top - openName.bottom >= barHeight - 0.01f);
                CHECK(openRowBox.bottom - gauge.bottom >= barHeight - 0.01f);
            }
            Unload(fixture, list);

            // Neither the loads shown nor the group's own names change the width: the sizer row has every text.
            ChannelListTexts calm = texts;
            calm.count = LongChannelRow + 1;
            calm.load.clear();
            CHECK(RowWidth(fixture, theme, calm) == doctest::Approx(width));
            ChannelListTexts otherGroup = calm;
            otherGroup.longName.clear();
            CHECK(RowWidth(fixture, theme, otherGroup) == doctest::Approx(width));

            // A load text too long for the names' width widens the rows, so its line keeps segments.
            ChannelListTexts longLoad = otherGroup;
            longLoad.load = LongLoadText;
            longLoad.sizerLoads = {LongLoadText};
            const ChannelList wide = LoadChannelList(fixture, theme, longLoad);
            CHECK(BorderWidth(*wide.rows.front()) > width + Tolerance);
            CHECK(CoveredByLoadText(*wide.rows[LongChannelRow]).first >= 1);
            Unload(fixture, wide);
        }
    }
}
