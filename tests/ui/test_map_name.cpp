#include "stdafx.h"
#include <doctest.h>
#include "RmlLayoutFixture.h"
#include "Core/Utilities/StringUtils.h"
#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"
#include "UI/HUD/UIMapName.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/ThemeFonts.h"
#include "UI/RmlBridge/ThemeInheritance.h"
#include "World/MapInfra/MapManager.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/FontEffect.h>
#include <RmlUi/Core/FontGlyph.h>
#include <array>
#include <climits>
#include <filesystem>
#include <string>
#include <utility>

namespace
{
constexpr float PixelTolerance = 1.f;

class MapNameProbe : public CUIMapName
{
public:
    float Alpha() const { return m_fAlpha; }
    const UI::MapName::MapNameRmlModel& Model() const { return m_RmlView.GetModel(); }
    Rml::ElementDocument* Document() { return m_RmlView.Document(); }
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
        : m_Directory(std::filesystem::current_path()), m_Theme(UI::RmlBridge::GetActiveThemeName()),
          m_Locale(I18N::GetCurrentLocale()), m_World(gMapManager.WorldActive),
          m_NonPvp(g_ServerListManager->m_byNonPvP)
    {
        std::filesystem::current_path(MU_BIN_DIR);
        UI::RmlBridge::ClearThemeFontCache();
        UI::RmlBridge::SetContextResolver([this](std::string_view) { return context; });
        REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/NotoSansTC-Regular.otf", true));
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

void CheckTextEffects(Rml::Element& text)
{
    const auto* property = text.GetProperty(Rml::PropertyId::FontEffect);
    REQUIRE(property != nullptr);
    const auto effects = property->Get<Rml::FontEffectsPtr>();
    REQUIRE(effects != nullptr);
    REQUIRE(effects->list.size() == 2);
    constexpr int probeSize = 10;
    constexpr int edgeWidth = 2;
    Rml::FontGlyph glyph;
    for (const auto& effect : effects->list)
    {
        Rml::Vector2i origin(0, 0), dimensions(probeSize, probeSize);
        REQUIRE(effect->GetGlyphMetrics(origin, dimensions, glyph));
        if (effect->HasUniqueTexture())
        {
            CHECK(origin == Rml::Vector2i(-edgeWidth, -edgeWidth));
            CHECK(dimensions == Rml::Vector2i(probeSize + 2 * edgeWidth, probeSize + 2 * edgeWidth));
        }
        else
        {
            CHECK(origin == Rml::Vector2i(edgeWidth, edgeWidth));
            CHECK(dimensions == Rml::Vector2i(probeSize, probeSize));
        }
    }
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
    for (int current : std::array<int, 4>{WD_41CHANGEUP3RD_1ST, WD_42CHANGEUP3RD_2ND,
                                        WD_45CURSEDTEMPLE_LV1, WD_0LORENCIA})
    {
        gMapManager.WorldActive = current;
        CheckName(WD_0LORENCIA, I18N::Game::Lorencia);
        CheckName(WD_10HEAVEN, I18N::Game::Icarus);
        CheckName(WD_41CHANGEUP3RD_1ST, I18N::Game::BalgassBarrack);
        CheckName(WD_42CHANGEUP3RD_2ND, I18N::Game::BalgassRestingPlace);
        for (int id : {9, 32}) CheckName(id, I18N::Game::DevilSquare);
        for (int id : {11, 17, 52}) CheckName(id, I18N::Game::BloodCastle);
        for (int id : {18, 23, 53}) CheckName(id, I18N::Game::ChaosCastle);
        for (int id : {24, 29}) CheckName(id, I18N::Game::Kalima);
        CheckName(36, I18N::Game::LostKalima);
        for (int id : {45, 50}) CheckName(id, I18N::Game::IllusionTemple);
        for (int id : {65, 68}) CheckName(id, I18N::Game::Doppelganger);
        for (int id : {69, 72}) CheckName(id, I18N::Game::Varka);
        CheckName(37, I18N::Game::Kanturu);
        CheckName(38, I18N::Game::Kanturu3);
        CheckName(39, I18N::Game::RefineryTower);
        for (int id : {80, 81}) CheckName(id, I18N::Game::Karutan);
        for (int id : {INT_MIN, -1, 43, 54, 82, INT_MAX})
            CheckName(id, I18N::Game::UnknownMap);
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
}

TEST_CASE("map-name live binding survives locale and theme changes as plain text [ui][map-name]")
{
    MapNameFixture fixture;
    MapNameProbe banner;
    banner.Init();
    gMapManager.WorldActive = WD_38KANTURU_2ND;
    banner.ShowMapName();
    banner.Render();
    I18N::SetLocale("pl");
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

TEST_CASE("map-name text wraps above a shared lower ornament across themes and scales [ui][map-name]")
{
    MapNameFixture fixture;
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
        for (Rml::Vector2i viewport : {Rml::Vector2i(800, 600), {1280, 720}, {1920, 1080},
                                     {2560, 1440}, {3440, 1440}, {3840, 2160}})
        for (const char* name : {"Lorencia", "The Extremely Long Forgotten Refinery Tower", "Żółć Привет", "冰風谷",
                                 "AnExceptionallyLongUnbrokenMapNameForWrapping"})
        for (bool warning : {false, true})
        {
            CAPTURE(theme);
            CAPTURE(percent);
            CAPTURE(viewport.x);
            CAPTURE(viewport.y);
            CAPTURE(name);
            CAPTURE(warning);
            fixture.Configure(viewport, percent);
            banner.SetText(name);
            banner.SetWarning(warning);
            fixture.Refresh();
            auto* document = banner.Document();
            auto* frame = document->GetElementById("banner");
            auto* text = document->GetElementById("name_text");
            auto* middle = document->GetElementById("name");
            CheckBounds(*frame, *document);
            CheckBounds(*text, *middle);
            CheckBounds(*middle, *frame);
            CHECK(document->GetElementById("ornament_top") == nullptr);
            CheckBounds(*document->GetElementById("ornament_bottom"), *frame);
            Rml::Vector2f position, size;
            REQUIRE(UI::RmlBridge::DrawnBox(*frame, Rml::BoxArea::Border, position, size));
            CHECK(position.x + size.x / 2.f == doctest::Approx(viewport.x / 2.f).epsilon(0.002));
            if (warning)
            {
                Rml::Vector2f warningPosition, warningSize;
                REQUIRE(UI::RmlBridge::DrawnBox(*document->GetElementById("strife"),
                    Rml::BoxArea::Border, warningPosition, warningSize));
                CHECK(warningPosition.y + warningSize.y <= position.y + PixelTolerance);
                CHECK(warningPosition.x + warningSize.x / 2.f ==
                      doctest::Approx(viewport.x / 2.f).epsilon(0.002));
            }
            fixture.context->ProcessMouseMove(static_cast<int>(position.x + size.x / 2.f),
                                             static_cast<int>(position.y + size.y / 2.f), 0);
            CHECK_FALSE(UI::RmlBridge::IsPointerOver(frame));
            CHECK(text->GetComputedValues().font_size() > 0.f);
            CHECK(text->GetComputedValues().font_weight() == Rml::Style::FontWeight::Normal);
            CHECK(text->GetComputedValues().color() == Rml::Colourb(240, 236, 226));
            CheckTextEffects(*text);
        }
    }
}

TEST_CASE("map-name italic-only optional family and missing-font fallback stay paired [ui][map-name]")
{
    MapNameFixture fixture;
    const UI::RmlBridge::ThemeInheritance theme("Data/Interface/RmlUi/themes/legacy");
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name") ==
          std::optional<std::string>("Liberation Sans"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name-style") ==
          std::optional<std::string>("normal"));

    // Emulate an italic-only installed face with a redistributable fixture font.
    // This regression runs on machines without the optional commercial font.
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/LiberationSans-Regular.ttf",
        "Monotype Corsiva", Rml::Style::FontStyle::Italic, Rml::Style::FontWeight::Normal));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name") ==
          std::optional<std::string>("Monotype Corsiva"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name-style") ==
          std::optional<std::string>("italic"));
}

TEST_CASE("map-name configured optional file uses its real style or bundled fallback [ui][map-name]")
{
    MapNameFixture fixture;
    const UI::RmlBridge::ThemeInheritance theme("Data/Interface/RmlUi/themes/legacy");
    UI::RmlBridge::LoadThemeFonts(theme);
    const auto family = UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name");
    const auto style = UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name-style");
    auto* engine = Rml::GetFontEngineInterface();
    REQUIRE(engine != nullptr);
    if (family == std::optional<std::string>("Monotype Corsiva"))
    {
        CHECK(style == std::optional<std::string>("italic"));
        CHECK(engine->GetFontFaceHandle("monotype corsiva", Rml::Style::FontStyle::Italic,
            Rml::Style::FontWeight::Normal, 18) != 0);
        CHECK(engine->GetFontFaceHandle("monotype corsiva", Rml::Style::FontStyle::Normal,
            Rml::Style::FontWeight::Normal, 18) == 0);
        MESSAGE("Configured file loaded: Monotype Corsiva, italic, weight400 (not a substituted fixture font)");
    }
    else
    {
        CHECK(family == std::optional<std::string>("Liberation Sans"));
        CHECK(style == std::optional<std::string>("normal"));
        CHECK(engine->GetFontFaceHandle("liberation sans", Rml::Style::FontStyle::Normal,
            Rml::Style::FontWeight::Normal, 18) != 0);
        MESSAGE("Optional font absent: bundled Liberation Sans, normal, weight400");
    }
}
