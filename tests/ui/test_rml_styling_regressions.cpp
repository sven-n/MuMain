#include "stdafx.h"
#include <doctest.h>

#include "RmlLayoutFixture.h"
#include "../../src/ThirdParty/RmlUi/Source/Core/DecoratorTiled.h"
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/TextShapingContext.h>
#include <algorithm>

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
}

TEST_CASE("atlas repetition clips partial tiles without sampling adjacent sprites [ui][styling]")
{
    UI::Tests::RmlLayoutFixture fixture;
    auto* document = fixture.LoadMarkup(R"(<rml><body><div id="panel"></div></body></rml>)");
    REQUIRE(document);
    auto* panel = document->GetElementById("panel");
    for (auto mode : {Rml::DecoratorTiled::REPEAT, Rml::DecoratorTiled::REPEAT_X, Rml::DecoratorTiled::REPEAT_Y})
    {
        CAPTURE(mode);
        Rml::Mesh mesh;
        auto tile = AtlasTile(mode);
        tile.GenerateGeometry(mesh, panel->GetComputedValues(), {}, {19.f, 14.f}, {8.f, 6.f});
        const size_t quads = mode == Rml::DecoratorTiled::REPEAT ? 9 : 3;
        CHECK(mesh.vertices.size() == quads * 4);
        CHECK(mesh.indices.size() == quads * 6);
        for (const auto& vertex : mesh.vertices)
        {
            CHECK(vertex.position.x >= 0.f);
            CHECK(vertex.position.x <= 19.f);
            CHECK(vertex.position.y >= 0.f);
            CHECK(vertex.position.y <= 14.f);
            CHECK(vertex.tex_coord.x >= 0.25f);
            CHECK(vertex.tex_coord.x <= 0.5f);
            CHECK(vertex.tex_coord.y >= 0.25f);
            CHECK(vertex.tex_coord.y <= 0.5f);
        }
        CHECK(mesh.vertices.back().position.y == (mode == Rml::DecoratorTiled::REPEAT_X ? 6.f : 14.f));
    }
}

TEST_CASE("a sheet sprite never samples the next sprite at its edge inside the sheet [ui][styling]")
{
    // The cash shop's zone tab: the upper 76 x 23 sprite of a 76 x 46 sheet, drawn at 1.6x. Its bottom edge lies on
    // the next sprite; its other edges are the texture border, which the renderer clamps.
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

    // The sprite and a band along its bottom edge; nothing splits it across.
    CHECK(mesh.vertices.size() == 8);
    float height = 0.f;
    for (const auto& vertex : mesh.vertices)
        height = std::max(height, vertex.position.y);
    const float lastRowCentre = 22.5f / 46.f;
    for (const auto& vertex : mesh.vertices)
    {
        CAPTURE(vertex.position.y);
        // Mapped as one quad would be, but never past the last row's centre.
        CHECK(vertex.tex_coord.y == doctest::Approx(std::min(vertex.position.y / height * 0.5f, lastRowCentre)));
        CHECK((vertex.tex_coord.x == 0.f || vertex.tex_coord.x == 1.f));
    }

    // A sprite without an edge inside a sheet stays one quad.
    Rml::Mesh whole;
    tile.tile_data.texcoords[1] = {1.f, 1.f};
    tile.GenerateGeometry(whole, panel->GetComputedValues(), {}, {121.6f, 36.8f}, {121.6f, 36.8f});
    CHECK(whole.vertices.size() == 4);
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
