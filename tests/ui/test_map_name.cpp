#include "stdafx.h"
#include <doctest.h>
#include "RmlLayoutFixture.h"
#include "Core/Utilities/StringUtils.h"
#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "UI/HUD/MapNameOrnament.h"
#include "UI/HUD/UIMapName.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlNativeTextFit.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/ThemeFonts.h"
#include "UI/RmlBridge/ThemeInheritance.h"
#include "World/MapInfra/MapManager.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/FontEffect.h>
#include <RmlUi/Core/FontGlyph.h>
#include <RmlUi/Core/TextShapingContext.h>
#include <SDL3/SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <utility>

namespace
{
constexpr float PixelTolerance = 1.f;
constexpr GLuint GensWarningBitmap = BITMAP_INTERFACE_EX + 47;

class WarningTextureFixture
{
public:
    ~WarningTextureFixture()
    {
        Bitmaps.UnloadImage(GensWarningBitmap, true);
        mu::ShutdownSDLGpuRenderer();
        SDL_DestroyWindow(window);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        std::error_code error;
        std::filesystem::remove(directory / "warning.ozt", error);
        std::filesystem::remove(directory, error);
    }
    SDL_Window* window = nullptr;
    std::filesystem::path directory =
        std::filesystem::temp_directory_path() / ("mu-map-name-gens-" + std::to_string(timeGetTime()));
};

class MapNameProbe : public CUIMapName
{
public:
    float Alpha() const
    {
        return m_fAlpha;
    }
    const UI::MapName::MapNameRmlModel& Model() const
    {
        return m_RmlView.GetModel();
    }
    Rml::ElementDocument* Document()
    {
        return m_RmlView.Document();
    }
    void SetText(Rml::String text)
    {
        m_RmlView.GetModel().mapName = std::move(text);
        m_RmlView.MarkDirty("map_name");
    }
    void SetWarning(bool visible)
    {
        m_RmlView.GetModel().strife = visible;
        m_RmlView.MarkDirty("strife");
    }
};

class MapNameFixture : public UI::Tests::RmlLayoutFixture
{
public:
    MapNameFixture()
        : m_Directory(std::filesystem::current_path()), m_Locale(I18N::GetCurrentLocale()),
          m_World(gMapManager.WorldActive), m_NonPvp(g_ServerListManager->m_byNonPvP)
    {
        std::filesystem::current_path(MU_BIN_DIR);
        m_Theme = UI::RmlBridge::GetActiveThemeName();
        UI::RmlBridge::ClearThemeFontCache();
        UI::RmlBridge::SetContextResolver([this](std::string_view) { return context; });
        REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/NotoSansTC-Regular.otf", true));
        REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/NotoSansJP-Regular.otf"));
        I18N::SetLocale("en");
        g_ServerListManager->m_byNonPvP = 0;
    }

    ~MapNameFixture()
    {
        UI::RmlBridge::SetContextResolver({});
        UI::RmlBridge::SetActiveThemeName(m_Theme);
        I18N::SetLocale(m_Locale.c_str());
        gMapManager.WorldActive = m_World;
        g_ServerListManager->m_byNonPvP = m_NonPvp;
        std::filesystem::current_path(m_Directory);
    }

private:
    std::filesystem::path m_Directory;
    std::string m_Theme;
    std::string m_Locale;
    int m_World;
    BYTE m_NonPvp;
};

void CheckName(int mapId, const wchar_t* expected)
{
    CAPTURE(mapId);
    CHECK(std::wstring(gMapManager.GetMapName(mapId)) == expected);
}

// One screen-pixel outline keeps contrast without introducing a blurred halo.
void CheckTextEffects(Rml::Element& text)
{
    const auto* property = text.GetProperty(Rml::PropertyId::FontEffect);
    REQUIRE(property != nullptr);
    const auto effects = property->Get<Rml::FontEffectsPtr>();
    REQUIRE(effects != nullptr);
    REQUIRE(effects->list.size() == 1);
    Rml::FontGlyph glyph;
    Rml::Vector2i origin(0, 0), dimensions(10, 10);
    REQUIRE(effects->list[0]->GetGlyphMetrics(origin, dimensions, glyph));
    CHECK(effects->list[0]->GetLayer() == Rml::FontEffect::Layer::Back);
    CHECK(origin == Rml::Vector2i(-2, -2));
    CHECK(dimensions == Rml::Vector2i(14, 14));
}

// The width of the name's glyphs as drawn. The text-fit span always spans the room, so its own box
// says nothing about whether the name fits.
float GlyphRunWidth(Rml::Element& text)
{
    auto* node = rmlui_dynamic_cast<Rml::ElementText*>(text.GetFirstChild());
    REQUIRE(node != nullptr);
    const auto& values = text.GetComputedValues();
    const Rml::TextShapingContext shaping{values.language(), values.direction(), values.font_kerning(),
                                          values.letter_spacing()};
    REQUIRE(text.GetFontFaceHandle() != 0);
    return static_cast<float>(
        Rml::GetFontEngineInterface()->GetStringWidth(text.GetFontFaceHandle(), node->GetText(), shaping));
}

void CheckSharpText(Rml::Element& text)
{
    Rml::Vector2f position, size;
    REQUIRE(UI::RmlBridge::DrawnBox(text, Rml::BoxArea::Border, position, size));
    const auto layout = text.GetBox().GetSize(Rml::BoxArea::Border);
    CHECK(size.x == doctest::Approx(layout.x).epsilon(0.002));
    CHECK(size.y == doctest::Approx(layout.y).epsilon(0.002));
}

// The silver's light top: the same name in white, without the edge, faded out by a mask.
void CheckShine(Rml::Element& shine, Rml::Element& text)
{
    CHECK(shine.GetComputedValues().font_size() == text.GetComputedValues().font_size());
    CHECK(shine.GetComputedValues().color() == Rml::Colourb(250, 250, 250));
    const auto* effects = shine.GetProperty(Rml::PropertyId::FontEffect);
    CHECK((effects == nullptr || effects->Get<Rml::FontEffectsPtr>() == nullptr ||
           effects->Get<Rml::FontEffectsPtr>()->list.empty()));
    const auto* mask = shine.GetProperty(Rml::PropertyId::MaskImage);
    REQUIRE(mask != nullptr);
    CHECK(mask->ToString().find("linear-gradient") != Rml::String::npos);
}

// Where the name's glyphs stand: the baseline of its one line, in drawn pixels.
float DrawnBaseline(Rml::Element& text)
{
    Rml::Vector2f position, size;
    REQUIRE(UI::RmlBridge::DrawnBox(text, Rml::BoxArea::Content, position, size));
    REQUIRE(text.GetFontFaceHandle() != 0);
    const auto& metrics = Rml::GetFontEngineInterface()->GetFontMetrics(text.GetFontFaceHandle());
    return position.y + (text.GetLineHeight() - (metrics.ascent + metrics.descent)) * 0.5f + metrics.ascent;
}

void CheckBounds(Rml::Element& child, Rml::Element& parent)
{
    Rml::Vector2f position, size, parentPosition, parentSize;
    REQUIRE(UI::RmlBridge::DrawnBox(child, Rml::BoxArea::Border, position, size));
    REQUIRE(UI::RmlBridge::DrawnBox(parent, Rml::BoxArea::Border, parentPosition, parentSize));
    CHECK(position.x >= parentPosition.x - PixelTolerance);
    CHECK(position.y >= parentPosition.y - PixelTolerance);
    CHECK(position.x + size.x <= parentPosition.x + parentSize.x + PixelTolerance);
    CHECK(position.y + size.y <= parentPosition.y + parentSize.y + PixelTolerance);
}
} // namespace

TEST_CASE("map names use the argument and bounded event aliases [ui][map-name]")
{
    MapNameFixture fixture;
    for (int current :
         std::array<int, 8>{WD_41CHANGEUP3RD_1ST, WD_42CHANGEUP3RD_2ND, WD_45CURSEDTEMPLE_LV1, WD_0LORENCIA,
                            WD_11BLOODCASTLE1, WD_18CHAOS_CASTLE, WD_24HELLAS, WD_24HELLAS_7})
    {
        gMapManager.WorldActive = current;
        CheckName(WD_0LORENCIA, I18N::Game::Lorencia);
        CheckName(WD_10HEAVEN, I18N::Game::Icarus);
        CheckName(WD_41CHANGEUP3RD_1ST, I18N::Game::BalgassBarrack);
        CheckName(WD_42CHANGEUP3RD_2ND, I18N::Game::BalgassRestingPlace);
        for (int id : {9, 32})
            CheckName(id, I18N::Game::DevilSquare);
        for (int id : {11, 17, 52})
            CheckName(id, I18N::Game::BloodCastle);
        for (int id : {18, 23, 53})
            CheckName(id, I18N::Game::ChaosCastle);
        for (int id : {24, 29})
            CheckName(id, I18N::Game::Kalima);
        CheckName(36, I18N::Game::LostKalima);
        for (int id : {34, 35})
            CheckName(id, I18N::Game::CrywolfFortress);
        for (int id : {45, 50})
            CheckName(id, I18N::Game::IllusionTemple);
        for (int id : {65, 68})
            CheckName(id, I18N::Game::Doppelganger);
        for (int id : {69, 72})
            CheckName(id, I18N::Game::Varka);
        CheckName(37, I18N::Game::Kanturu);
        CheckName(38, I18N::Game::Kanturu3);
        CheckName(38, L"Kanturu Relics");
        CheckName(39, I18N::Game::RefineryTower);
        for (int id : {80, 81})
            CheckName(id, I18N::Game::Karutan);
        for (int id : {INT_MIN, -1, 43, 54, 82, INT_MAX})
        {
            CheckName(id, I18N::Game::UnknownMap);
            CHECK(gMapManager.FindMapName(id) == nullptr);
        }
    }
}

TEST_CASE("same-map activation resets fade and refreshes Gens without artwork [ui][map-name]")
{
    MapNameFixture fixture;
    MapNameProbe banner;
    banner.Init();
    gMapManager.WorldActive = WD_63PK_FIELD;
    banner.ShowMapName();
    banner.Render();
    CHECK(banner.Model().strife);
    CHECK(banner.Model().strifeSource.empty());
    g_ServerListManager->m_byNonPvP = 1;
    banner.ShowMapName();
    CHECK(banner.Alpha() == doctest::Approx(0.2f));
    banner.Render();
    CHECK_FALSE(banner.Model().strife);
    CHECK(banner.Model().strifeSource.empty());
    g_ServerListManager->m_byNonPvP = 0;
    banner.ShowMapName();
    banner.Render();
    REQUIRE(banner.Model().strife);
    gMapManager.WorldActive = WD_0LORENCIA;
    banner.ShowMapName();
    banner.Render();
    REQUIRE(banner.Document() != nullptr);
    CHECK(banner.Document()->IsVisible());
    CHECK(banner.Model().mapName == "Lorencia");
    CHECK_FALSE(banner.Model().strife);
    CHECK(banner.Model().strifeSource.empty());
    banner.ShowMapName();
    CHECK(banner.Alpha() == doctest::Approx(0.2f));
    gMapManager.WorldActive = WD_40AREA_FOR_GM;
    banner.ShowMapName();
    banner.Render();
    REQUIRE(banner.Document() != nullptr);
    CHECK_FALSE(banner.Document()->IsVisible());
    for (int mapId : {INT_MIN, -1, 43, 54, 82, INT_MAX})
    {
        CAPTURE(mapId);
        gMapManager.WorldActive = mapId;
        banner.ShowMapName();
        banner.Render();
        CHECK_FALSE(banner.Document()->IsVisible());
    }
    gMapManager.WorldActive = WD_35CRYWOLF_2ND;
    banner.ShowMapName();
    banner.Render();
    CHECK(banner.Document()->IsVisible());
    CHECK(banner.Model().mapName == StringUtils::WideToNarrow(I18N::Game::CrywolfFortress));
}

TEST_CASE("map-name live binding survives locale and theme changes as plain text [ui][map-name]")
{
    MapNameFixture fixture;
    MapNameProbe banner;
    banner.Init();
    gMapManager.WorldActive = WD_38KANTURU_2ND;
    banner.ShowMapName();
    banner.Render();
    const auto englishName = banner.Model().mapName;
    I18N::SetLocale("ja");
    REQUIRE(StringUtils::WideToNarrow(I18N::Game::Kanturu3) != englishName);
    CHECK(banner.Model().mapName == StringUtils::WideToNarrow(I18N::Game::Kanturu3));
    for (const char* theme : {"legacy", "modern"})
    {
        UI::RmlBridge::SetActiveThemeName(theme);
        UI::RmlBridge::ReloadAllThemedDocuments();
        banner.Render();
        banner.SetText("<b>Lorencia & Żółć</b>");
        fixture.Refresh();
        auto* text = banner.Document()->GetElementById("name_text");
        REQUIRE(text != nullptr);
        CHECK(text->GetNumChildren() == 1);
        auto* node = rmlui_dynamic_cast<Rml::ElementText*>(text->GetFirstChild());
        REQUIRE(node != nullptr);
        CHECK(node->GetText() == "<b>Lorencia & Żółć</b>");
    }
}

TEST_CASE("map-name Gens texture is retried after loading [ui][map-name][gpu]" *
          doctest::skip(std::getenv("MU_RUN_GPU_UPLOAD_TESTS") == nullptr))
{
    MapNameFixture fixture;
    WarningTextureFixture texture;
    REQUIRE(SDL_InitSubSystem(SDL_INIT_VIDEO));
    texture.window = SDL_CreateWindow("map-name Gens test", 64, 64, SDL_WINDOW_HIDDEN);
    REQUIRE(texture.window != nullptr);
    REQUIRE(mu::InitSDLGpuRenderer(texture.window, "DejaVu Sans", "default", 12.f, 14.f, 12.f));
    MapNameProbe banner;
    banner.Init();
    gMapManager.WorldActive = WD_63PK_FIELD;
    banner.ShowMapName();
    banner.Render();
    REQUIRE(banner.Model().strife);
    REQUIRE(banner.Model().strifeSource.empty());
    REQUIRE(std::filesystem::create_directory(texture.directory));
    // A one-pixel OZT exercises the production cache without requiring localized game artwork.
    std::array<unsigned char, 26> pixels{};
    pixels[16] = pixels[18] = 1;
    pixels[20] = 32;
    std::fill(pixels.begin() + 22, pixels.end(), 255);
    {
        std::ofstream image(texture.directory / "warning.ozt", std::ios::binary);
        image.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
        REQUIRE(image.good());
    }
    const auto warningPath = texture.directory / "warning.tga";
    REQUIRE(Bitmaps.LoadImage(GensWarningBitmap, warningPath.wstring()));
    banner.Render();
    CHECK(banner.Model().strifeSource == "/" + warningPath.generic_string());
    gMapManager.WorldActive = WD_0LORENCIA;
    banner.ShowMapName();
    banner.Render();
    CHECK_FALSE(banner.Model().strife);
    CHECK(banner.Model().strifeSource.empty());
}

TEST_CASE("map-name text uses final-size rasters without deformation across themes and scales [ui][map-name]")
{
    MapNameFixture fixture;
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/MuMapName-Italic.otf"));
    MapNameProbe banner;
    banner.Init();
    gMapManager.WorldActive = WD_0LORENCIA;
    banner.ShowMapName();
    banner.Render();
    for (const char* theme : {"legacy", "modern"})
    {
        UI::RmlBridge::SetActiveThemeName(theme);
        UI::RmlBridge::ReloadAllThemedDocuments();
        for (int percent : {75, 100, 125, 150})
            for (Rml::Vector2i viewport : {Rml::Vector2i(800, 600),
                                           {1280, 720},
                                           {1366, 768},
                                           {1600, 900},
                                           {1920, 1080},
                                           {2560, 1440},
                                           {3440, 1440},
                                           {3840, 2160}})
            {
                float fullSizeBaseline = 0.f; // Lorencia's, the first name: it is never shrunk
                for (const char* name :
                     {"Lorencia", "Balgass' Resting Place", "The Extremely Long Forgotten Refinery Tower",
                      "Żółć Привет", "冰風谷", "AnExceptionallyLongUnbrokenMapNameForWrapping"})
                    for (bool warning : {false, true})
                    {
                        CAPTURE(theme);
                        CAPTURE(percent);
                        CAPTURE(viewport.x);
                        CAPTURE(viewport.y);
                        CAPTURE(name);
                        CAPTURE(warning);
                        fixture.Configure(viewport, percent);
                        // Render() runs the banner's real-time fade; showing it again keeps a slow run from
                        // hiding the document, which RmlUi then stops laying out.
                        banner.ShowMapName();
                        banner.SetText(name);
                        fixture.Refresh();
                        UI::RmlBridge::FitNativeTextToBoxes(fixture.context); // same post-layout pass as the runtime
                        banner.SetWarning(warning);
                        fixture.Refresh();
                        auto* document = banner.Document();
                        auto* frame = document->GetElementById("banner");
                        auto* text = document->GetElementById("name_text");
                        const float dp = fixture.context->GetDensityIndependentPixelRatio();
                        CheckBounds(*frame, *document);
                        Rml::Vector2f position, size;
                        REQUIRE(UI::RmlBridge::DrawnBox(*frame, Rml::BoxArea::Border, position, size));
                        CHECK(position.x + size.x / 2.f == doctest::Approx(viewport.x / 2.f).epsilon(0.002));
                        CHECK(std::abs(size.x - std::min(384.f * dp, viewport.x * 0.9f)) <= PixelTolerance);
                        CHECK(std::abs(size.y - 90.f * dp) <= PixelTolerance);
                        for (const char* piece : {"swirl_left", "swirl_right", "icon"})
                        {
                            CAPTURE(piece);
                            REQUIRE(document->GetElementById(piece) != nullptr);
                            CheckBounds(*document->GetElementById(piece), *frame);
                        }
                        // Fitting changes the raster font size; the drawn glyph geometry stays at 1:1.
                        Rml::Vector2f textPosition, textSize;
                        REQUIRE(UI::RmlBridge::DrawnBox(*text, Rml::BoxArea::Border, textPosition, textSize));
                        CHECK(GlyphRunWidth(*text) <=
                              document->GetElementById("name_room")->GetOffsetWidth() + PixelTolerance);
                        CHECK(textPosition.x + textSize.x / 2.f == doctest::Approx(viewport.x / 2.f).epsilon(0.002));
                        CHECK(textPosition.y + textSize.y <= position.y + size.y);
                        // A shrunk name stays on the full-size baseline instead of sinking toward the swirls.
                        if (std::string_view(name) == "Lorencia")
                            fullSizeBaseline = DrawnBaseline(*text);
                        CHECK(std::abs(DrawnBaseline(*text) - fullSizeBaseline) <= PixelTolerance);
                        if (warning)
                        {
                            Rml::Vector2f warningPosition, warningSize;
                            REQUIRE(UI::RmlBridge::DrawnBox(*document->GetElementById("strife"), Rml::BoxArea::Border,
                                                            warningPosition, warningSize));
                            CHECK(warningPosition.y + warningSize.y <= position.y + PixelTolerance);
                            CHECK(warningPosition.x + warningSize.x / 2.f ==
                                  doctest::Approx(viewport.x / 2.f).epsilon(0.002));
                            CHECK(std::abs(warningSize.x - 166.f * dp) <= PixelTolerance);
                            CHECK(std::abs(warningSize.y - 28.f * dp) <= PixelTolerance);
                        }
                        fixture.context->ProcessMouseMove(static_cast<int>(position.x + size.x / 2.f),
                                                          static_cast<int>(position.y + size.y / 2.f), 0);
                        CHECK_FALSE(UI::RmlBridge::IsPointerOver(frame));
                        CHECK(text->GetComputedValues().font_size() <= 50.f * dp + 0.01f);
                        if (std::string_view(name) == "Lorencia" || std::string_view(name) == "Balgass' Resting Place")
                            CHECK(text->GetComputedValues().font_size() == doctest::Approx(50.f * dp));
                        CheckSharpText(*text);
                        CHECK(text->GetComputedValues().font_family() == "mu map name");
                        CHECK(UI::RmlBridge::DrawnScale(*frame) == doctest::Approx(1.f).epsilon(0.002));
                        CHECK(text->GetComputedValues().font_weight() == Rml::Style::FontWeight::Normal);
                        CHECK(text->GetComputedValues().color() == Rml::Colourb(142, 139, 134));
                        CheckTextEffects(*text);
                        REQUIRE(document->GetElementById("name_shine") != nullptr);
                        CheckShine(*document->GetElementById("name_shine"), *text);
                    }
            }
    }
}

TEST_CASE("map-name fitting follows same-name typography and room changes [ui][map-name]")
{
    MapNameFixture fixture;
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/MuMapName-Italic.otf"));
    MapNameProbe banner;
    banner.Init();
    gMapManager.WorldActive = WD_0LORENCIA;
    banner.ShowMapName();
    banner.Render();
    banner.SetText("The Extremely Long Forgotten Refinery Tower");
    fixture.Refresh();
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
    fixture.context->Render(); // RmlUi resolves transform matrices when drawing.
    auto* document = banner.Document();
    auto* text = document->GetElementById("name_text");
    auto* room = document->GetElementById("name_room");
    auto* layer = document->GetElementById("name");
    REQUIRE(text != nullptr);
    REQUIRE(room != nullptr);
    REQUIRE(layer != nullptr);
    const float originalFit = text->GetComputedValues().font_size();
    for (const char* id : {"name", "name_shine"})
        REQUIRE(document->GetElementById(id)->SetProperty("letter-spacing", "4px"));
    fixture.Refresh();
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
    fixture.context->Render(); // RmlUi resolves transform matrices when drawing.
    CHECK(text->GetComputedValues().font_size() < originalFit);
    CheckSharpText(*text);
    Rml::Vector2f position, size;
    REQUIRE(UI::RmlBridge::DrawnBox(*text, Rml::BoxArea::Border, position, size));
    CHECK(GlyphRunWidth(*text) <= room->GetOffsetWidth() + PixelTolerance);
    REQUIRE(room->SetProperty("right", "40px"));
    fixture.Refresh();
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
    fixture.context->Render(); // RmlUi resolves transform matrices when drawing.
    REQUIRE(UI::RmlBridge::DrawnBox(*text, Rml::BoxArea::Border, position, size));
    CHECK(GlyphRunWidth(*text) <= room->GetOffsetWidth() + PixelTolerance);
    banner.SetText("A");
    fixture.Refresh();
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
    fixture.context->Render(); // RmlUi resolves transform matrices when drawing.
    CHECK(text->GetComputedValues().font_size() == layer->GetComputedValues().font_size());
    CheckSharpText(*text);
}

TEST_CASE("map-name icon follows the map from the ornament files [ui][map-name]")
{
    MapNameFixture fixture;
    MapNameProbe banner;
    banner.Init();
    gMapManager.WorldActive = WD_0LORENCIA;
    banner.ShowMapName();
    CHECK(banner.Model().icon);
    CHECK(banner.Model().iconSource == "/Data/Interface/MapNameOrnament/lorencia.tga");
    CHECK(banner.Model().iconStyle == "left: 59dp; top: 46dp; width: 43dp; height: 36dp;");
    for (int map : {static_cast<int>(WD_11BLOODCASTLE1), 17, 52})
    {
        CAPTURE(map);
        gMapManager.WorldActive = map;
        banner.ShowMapName();
        CHECK(banner.Model().iconSource == "/Data/Interface/MapNameOrnament/bloodcastle.tga");
    }
    gMapManager.WorldActive = 43;
    banner.ShowMapName();
    CHECK_FALSE(banner.Model().icon);
}

TEST_CASE("map-name ornament files leave out what they do not describe [ui][map-name]")
{
    constexpr const char* layout = R"({"icons": {"lorencia": {"x": 59, "y": 46, "width": 43, "height": 36},
                                                 "broken": {"x": 1}}})";
    UI::MapName::MapNameOrnament ornament;
    REQUIRE(ornament.Read(layout, R"({"maps": [{"map": 0, "icon": "lorencia"}, {"map": 1, "icon": "missing"},
                                              {"map": 2, "icon": "broken"}, {"map": "3", "icon": "lorencia"}]})"));
    REQUIRE(ornament.FindIcon(0) != nullptr);
    CHECK(ornament.FindIcon(0)->image == "lorencia");
    CHECK(ornament.FindIcon(0)->width == doctest::Approx(43.f));
    for (int map : {1, 2, 3})
        CHECK(ornament.FindIcon(map) == nullptr);
    CHECK_FALSE(ornament.Read("{", R"({"maps": []})"));
    CHECK(ornament.FindIcon(0) == nullptr);
    CHECK_FALSE(ornament.Read(layout, "[]"));
}

TEST_CASE("map-name bundled italic family and missing-font fallback stay paired [ui][map-name]")
{
    for (const char* name : {"legacy", "modern"})
    {
        CAPTURE(name);
        MapNameFixture fixture;
        const UI::RmlBridge::ThemeInheritance theme(std::filesystem::path("Data/Interface/RmlUi/themes") / name);
        // Until the bundled face is loaded, the banner takes the theme's title family.
        CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name") ==
              UI::RmlBridge::ResolveThemeFontToken(theme, "font-title"));
        CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name-style") ==
              std::optional<std::string>("normal"));

        // The file the runtime loads with the other bundled fonts (RmlUiRuntime.cpp).
        REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/MuMapName-Italic.otf"));
        CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name") ==
              std::optional<std::string>("MU Map Name"));
        CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name-style") ==
              std::optional<std::string>("italic"));
    }
}

TEST_CASE("map-name bundled file is the italic MU Map Name face at normal weight [ui][map-name]")
{
    MapNameFixture fixture;
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/MuMapName-Italic.otf"));
    auto* engine = Rml::GetFontEngineInterface();
    REQUIRE(engine != nullptr);
    const auto face = [engine](Rml::Style::FontStyle style)
    { return engine->GetFontFaceHandle("mu map name", style, Rml::Style::FontWeight::Normal, 18); };
    CHECK(face(Rml::Style::FontStyle::Italic) != 0);
    CHECK(face(Rml::Style::FontStyle::Normal) == 0);
}

TEST_CASE("map-name Japanese face covers every Japanese map name [ui][map-name]")
{
    MapNameFixture fixture;
    I18N::SetLocale("ja");
    FT_Library rawLibrary = nullptr;
    REQUIRE(FT_Init_FreeType(&rawLibrary) == 0);
    std::unique_ptr<FT_LibraryRec_, decltype(&FT_Done_FreeType)> library(rawLibrary, FT_Done_FreeType);
    FT_Face rawFace = nullptr;
    const auto filename = std::string(MU_FONT_DIR) + "/NotoSansJP-Regular.otf";
    REQUIRE(FT_New_Face(library.get(), filename.c_str(), 0, &rawFace) == 0);
    std::unique_ptr<FT_FaceRec_, decltype(&FT_Done_Face)> face(rawFace, FT_Done_Face);
    for (int map = 0; map <= WD_81KARUTAN2; ++map)
    {
        const wchar_t* name = gMapManager.FindMapName(map);
        if (!name)
            continue;
        for (const wchar_t* character = name; *character; ++character)
        {
            CAPTURE(map);
            CAPTURE(static_cast<unsigned>(*character));
            CHECK(FT_Get_Char_Index(face.get(), *character) != 0);
        }
    }
}

TEST_CASE("Japanese map names use the Japanese face on the full-size baseline [ui][map-name]")
{
    MapNameFixture fixture;
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/MuMapName-Italic.otf"));
    MapNameProbe banner;
    banner.Init();
    gMapManager.WorldActive = WD_0LORENCIA;
    const auto show = [&](const char* locale)
    {
        I18N::SetLocale(locale);
        banner.ShowMapName();
        banner.Render();
        fixture.Refresh();
        UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
        fixture.Refresh();
        Rml::Element* text = banner.Document()->GetElementById("name_text");
        REQUIRE(text != nullptr);
        return text;
    };
    for (const char* theme : {"legacy", "modern"})
    {
        CAPTURE(theme);
        UI::RmlBridge::SetActiveThemeName(theme);
        UI::RmlBridge::ReloadAllThemedDocuments();
        Rml::Element* text = show("en");
        CHECK(text->GetComputedValues().font_family() == "mu map name");
        const float baseline = DrawnBaseline(*text);

        text = show("ja");
        CHECK(text->GetComputedValues().font_family() == "noto sans jp");
        CHECK(text->GetComputedValues().font_style() == Rml::Style::FontStyle::Normal);
        CHECK(std::abs(DrawnBaseline(*text) - baseline) <= PixelTolerance);

        text = show("zh-TW");
        CHECK(text->GetComputedValues().font_family() == "mu map name");
    }
}
