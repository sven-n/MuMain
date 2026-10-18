#include "stdafx.h"
#include <doctest.h>
#include "RmlLayoutFixture.h"
#include "RmlTextLayoutAudit.h"
#include "UI/MuHelper/MuHelperShared.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/Scaling/UITransform.h"
#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/ElementScroll.h>
#include <cctype>
#include <cstdlib>
#include <map>
#include <sstream>
#include <vector>

namespace
{
using UI::Tests::RmlLayoutFixture;
using UI::Tests::RmlTextLayoutAudit;
const Rml::Vector2i SmallViewport{1024, 768};
constexpr int SmallPercent = 75;
constexpr float NormalMinimum = 11.f;
constexpr float NormalMaximum = 16.f;
constexpr int ExpectedDocuments = 50;
constexpr size_t ExpectedScenarios = 1290;
constexpr float PixelTolerance = 1.f;
constexpr size_t BloodLevelCount = 8;
constexpr size_t DevilLevelCount = 7;

bool Hidden(const std::string& expression, int tab, const UI::MuHelper::ClassFeatures& features)
{
    if (expression == "active_tab == 0") return tab == 0;
    for (int index = 0; index < 3; ++index)
        if (expression == "active_tab != " + std::to_string(index)) return tab != index;
    const std::map<std::string, bool> flags = {{"skill3", features.skill3}, {"combo", features.combo},
        {"pet", features.pet}, {"party", features.party}, {"auto_heal", features.autoHeal},
        {"drain_life", features.drainLife}};
    constexpr const char* Prefix = "!features.";
    if (expression.starts_with(Prefix))
    {
        const auto it = flags.find(expression.substr(std::char_traits<char>::length(Prefix)));
        REQUIRE(it != flags.end());
        return !it->second;
    }
    REQUIRE(expression == "!show_still_opening");
    return false;
}

void ConfigureHelper(Rml::ElementDocument& document, int tab, int character)
{
    const auto features = UI::MuHelper::ResolveClassFeatures(character);
    Rml::ElementList elements;
    document.QuerySelectorAll(elements, "[data-audit-hidden], [data-audit-summoner]");
    for (auto* element : elements)
    {
        const auto expression = element->GetAttribute<Rml::String>("data-audit-hidden", "");
        if (!expression.empty()) element->SetClass("hidden", Hidden(expression, tab, features));
        if (element->HasAttribute("data-audit-summoner")) element->SetClass("summoner", features.potionSummoner);
    }
}

std::string NonWhitespace(const std::string& text)
{
    std::string result;
    for (unsigned char c : text)
        if (!std::isspace(c)) result += static_cast<char>(c);
    return result;
}

void ApplyNativeSizes(Rml::ElementDocument& document, Rml::Vector2i viewport)
{
    Rml::ElementList elements;
    document.QuerySelectorAll(elements, "[data-audit-font], [data-audit-native-root]");
    for (auto* element : elements)
    {
        if (element->HasAttribute("data-audit-native-root"))
            element->SetProperty("--text-px", std::to_string(UI::Scaling::NativeTextPixelSize(
                UI::Scaling::FontRole::Normal, viewport.x, viewport.y)) + "px");
        const auto role = element->GetAttribute<Rml::String>("data-audit-font", "normal") == "bold"
            ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        const auto value = std::to_string(UI::Scaling::NativeTextPixelSize(role, viewport.x, viewport.y)) + "px";
        if (element->HasAttribute("data-audit-font")) element->SetProperty("font-size", value);
    }
}

void CheckSources(Rml::ElementDocument& document)
{
    Rml::ElementList elements;
    document.QuerySelectorAll(elements, "[data-audit-source]");
    for (auto* element : elements)
    {
        auto* text = rmlui_dynamic_cast<Rml::ElementText*>(element->GetFirstChild());
        REQUIRE(text != nullptr);
        CHECK(text->GetText() == element->GetAttribute<Rml::String>("data-audit-source", ""));
        std::string wrapped;
        for (const auto& line : text->GetLines()) wrapped += line.text;
        CHECK(NonWhitespace(wrapped) == NonWhitespace(text->GetText()));
        CHECK(UI::RmlBridge::DrawnScale(*element) == doctest::Approx(1.f));
    }
}

void CheckScrollPane(RmlLayoutFixture& fixture, Rml::Element& pane)
{
    const bool needsScroll = pane.GetScrollHeight() > pane.GetClientHeight() + PixelTolerance;
    if (needsScroll)
    {
        auto* scrollbar = pane.GetElementScroll()->GetScrollbar(Rml::ElementScroll::VERTICAL);
        REQUIRE(scrollbar != nullptr);
        CHECK(scrollbar->GetComputedValues().visibility() == Rml::Style::Visibility::Visible);
        CHECK(scrollbar->GetBox().GetSize(Rml::BoxArea::Border).x > 0.f);
        CHECK(scrollbar->GetComputedValues().pointer_events() == Rml::Style::PointerEvents::Auto);
    }
    pane.SetScrollTop(pane.GetScrollHeight());
    fixture.Refresh();
    const float bottom = pane.GetAbsoluteOffset(Rml::BoxArea::Content).y + pane.GetClientHeight();
    Rml::Element* last = nullptr;
    for (int i = pane.GetNumChildren() - 1; i >= 0 && last == nullptr; --i)
        if (pane.GetChild(i)->IsVisible()) last = pane.GetChild(i);
    REQUIRE(last != nullptr);
    CHECK(last->GetAbsoluteOffset(Rml::BoxArea::Border).y + last->GetBox().GetSize(Rml::BoxArea::Border).y
          <= bottom + PixelTolerance);
    pane.SetScrollTop(0.f);
    fixture.Refresh();
}

void CheckLevelAccess(RmlLayoutFixture& fixture, Rml::Element& pane)
{
    Rml::ElementList buttons;
    pane.QuerySelectorAll(buttons, ".entry-btn");
    for (auto* button : buttons)
    {
        const float top = button->GetAbsoluteOffset(Rml::BoxArea::Border).y -
            pane.GetAbsoluteOffset(Rml::BoxArea::Content).y + pane.GetScrollTop();
        pane.SetScrollTop(top);
        fixture.Refresh();
        Rml::Vector2f position, size, panePosition, paneSize;
        REQUIRE(UI::RmlBridge::DrawnBox(*button, Rml::BoxArea::Border, position, size));
        REQUIRE(UI::RmlBridge::DrawnBox(pane, Rml::BoxArea::Content, panePosition, paneSize));
        CHECK(position.y >= panePosition.y - PixelTolerance);
        CHECK(position.y + size.y <= panePosition.y + paneSize.y + PixelTolerance);
        fixture.context->ProcessMouseMove(static_cast<int>(position.x + size.x / 2.f),
            static_cast<int>(position.y + size.y / 2.f), 0);
        CHECK(UI::RmlBridge::IsPointerOver(button));
    }
    pane.SetScrollTop(0.f);
    fixture.Refresh();
}

void CheckEvent(RmlLayoutFixture& fixture, Rml::ElementDocument& document, const std::string& window)
{
    auto* description = document.GetElementById("entry_description");
    auto* levels = document.GetElementById("entry_levels");
    REQUIRE(description != nullptr);
    REQUIRE(levels != nullptr);
    const auto viewport = fixture.context->GetDimensions();
    CHECK(description->GetComputedValues().font_size() == doctest::Approx(UI::Scaling::NativeTextPixelSize(
        UI::Scaling::FontRole::Normal, viewport.x, viewport.y)));
    CHECK(levels->GetComputedValues().font_size() == doctest::Approx(UI::Scaling::NativeTextPixelSize(
        UI::Scaling::FontRole::Bold, viewport.x, viewport.y)));
    Rml::ElementList fragments, buttons;
    description->QuerySelectorAll(fragments, ".entry-line");
    levels->QuerySelectorAll(buttons, ".entry-btn");
    const bool blood = window == "blood_castle_enter";
    CHECK(fragments.size() == (blood ? 1 : 6));
    CHECK(buttons.size() == (blood ? BloodLevelCount : DevilLevelCount));
    CheckSources(document);
    Rml::Vector2f titlePosition, titleSize, descriptionPosition, descriptionSize, levelsPosition, levelsSize, exitPosition, exitSize;
    REQUIRE(UI::RmlBridge::DrawnBox(*document.GetElementById("title"), Rml::BoxArea::Border, titlePosition, titleSize));
    REQUIRE(UI::RmlBridge::DrawnBox(*description, Rml::BoxArea::Border, descriptionPosition, descriptionSize));
    REQUIRE(UI::RmlBridge::DrawnBox(*levels, Rml::BoxArea::Border, levelsPosition, levelsSize));
    REQUIRE(UI::RmlBridge::DrawnBox(*document.GetElementById("btn_exit"), Rml::BoxArea::Border, exitPosition, exitSize));
    CHECK(titlePosition.y + titleSize.y <= descriptionPosition.y + PixelTolerance);
    CHECK(descriptionPosition.y + descriptionSize.y <= levelsPosition.y + PixelTolerance);
    CHECK(levelsPosition.y + levelsSize.y <= exitPosition.y + PixelTolerance);
    CheckScrollPane(fixture, *description);
    CheckScrollPane(fixture, *levels);
    CheckLevelAccess(fixture, *levels);
}

void CheckShop(RmlLayoutFixture& fixture, Rml::ElementDocument& document)
{
    auto* notice = document.GetElementById("shop_notice");
    REQUIRE(notice != nullptr);
    Rml::ElementList lines;
    notice->QuerySelectorAll(lines, ".notice-line");
    CHECK(lines.size() == 8);
    Rml::Vector2f noticePosition, noticeSize, exitPosition, exitSize;
    REQUIRE(UI::RmlBridge::DrawnBox(*notice, Rml::BoxArea::Border, noticePosition, noticeSize));
    REQUIRE(UI::RmlBridge::DrawnBox(*document.GetElementById("btn_exit"), Rml::BoxArea::Border, exitPosition, exitSize));
    CHECK(noticePosition.y + noticeSize.y <= exitPosition.y + PixelTolerance);
    if (auto* opening = document.GetElementById("still_opening_line"))
    {
        Rml::Vector2f openingPosition, openingSize;
        REQUIRE(UI::RmlBridge::DrawnBox(*opening, Rml::BoxArea::Border, openingPosition, openingSize));
        CHECK(openingPosition.y + openingSize.y <= noticePosition.y + PixelTolerance);
    }
    CheckScrollPane(fixture, *notice);
}

void CheckHelper(RmlLayoutFixture& fixture, Rml::ElementDocument& document)
{
    auto* body = document.GetElementById("mhc_body");
    auto* footer = document.GetElementById("mhc_footer");
    REQUIRE(body != nullptr);
    REQUIRE(footer != nullptr);
    Rml::Vector2f panelPosition, panelSize, bodyPosition, bodySize, footerPosition, footerSize;
    REQUIRE(UI::RmlBridge::DrawnBox(*document.GetElementById("panel"), Rml::BoxArea::Border, panelPosition, panelSize));
    REQUIRE(UI::RmlBridge::DrawnBox(*body, Rml::BoxArea::Border, bodyPosition, bodySize));
    REQUIRE(UI::RmlBridge::DrawnBox(*footer, Rml::BoxArea::Border, footerPosition, footerSize));
    CHECK(bodyPosition.y + bodySize.y <= footerPosition.y + PixelTolerance);
    CHECK(footerPosition.y + footerSize.y <= panelPosition.y + panelSize.y + PixelTolerance);
    CheckScrollPane(fixture, *body);
}

void RunScenario(RmlLayoutFixture& fixture, RmlTextLayoutAudit& audit, const std::string& markup,
                 const RmlTextLayoutAudit::Scenario& scenario, int character)
{
    CAPTURE(scenario.theme);
    CAPTURE(scenario.window);
    CAPTURE(scenario.locale);
    CAPTURE(scenario.width);
    CAPTURE(scenario.height);
    CAPTURE(scenario.percent);
    CAPTURE(scenario.contentScale);
    const Rml::Vector2i viewport{scenario.width, scenario.height};
    fixture.Configure(viewport, scenario.percent, scenario.contentScale);
    auto* document = fixture.LoadMarkup(markup);
    ConfigureHelper(*document, scenario.tab, character);
    UI::RmlBridge::SlotPlacement placement;
    placement.Set(0.f, 0.f, UI::Scaling::DockRightTransform(viewport.x, viewport.y).scaleX);
    placement.Apply(document, "panel");
    ApplyNativeSizes(*document, viewport);
    fixture.Refresh();
    if (scenario.window.ends_with("_enter")) CheckEvent(fixture, *document, scenario.window);
    if (scenario.window.ends_with("_shop")) CheckShop(fixture, *document);
    // Modern flows the MU Helper; legacy keeps native's positions.
    if (scenario.window == "mu_helper_config" && scenario.theme == "modern") CheckHelper(fixture, *document);
    const float expectedFloor = UI::Scaling::CachedFontPointSize(UI::Scaling::FontRole::Normal) * NormalMinimum / NormalMaximum;
    CHECK(UI::Scaling::MinimumTextPixelSize(UI::Scaling::FontRole::Normal) == doctest::Approx(expectedFloor));
    audit.Inspect(*document, scenario);
    fixture.context->UnloadDocument(document);
    fixture.Refresh();
}

void RunSweep(RmlLayoutFixture& fixture, RmlTextLayoutAudit& audit,
                      RmlTextLayoutAudit::Scenario scenario, const std::string& markup)
{
    for (Rml::Vector2i viewport : {SmallViewport, Rml::Vector2i{1280, 720}, Rml::Vector2i{1920, 1080}})
    for (int percent : {75, 100, 125, 150})
    for (float contentScale : {1.f, 1.5f, 2.f})
    {
        if ((viewport == SmallViewport) != (percent == SmallPercent)) continue;
        scenario.width = viewport.x;
        scenario.height = viewport.y;
        scenario.percent = percent;
        scenario.contentScale = contentScale;
        RunScenario(fixture, audit, markup, scenario, CLASS_KNIGHT);
    }
}

void RunDocument(RmlLayoutFixture& fixture, RmlTextLayoutAudit& audit,
                 RmlTextLayoutAudit::Scenario scenario, const std::string& markup)
{
    if (scenario.window.ends_with("_enter") || scenario.window.ends_with("_shop"))
    {
        RunSweep(fixture, audit, scenario, markup);
        return;
    }
    const bool helper = scenario.window == "mu_helper_config";
    scenario.width = SmallViewport.x;
    scenario.height = SmallViewport.y;
    scenario.percent = SmallPercent;
    const std::map<int, std::string> names{{CLASS_KNIGHT, "DarkKnight"}, {CLASS_WIZARD, "DarkWizard"},
        {CLASS_ELF, "Elf"}, {CLASS_DARK_LORD, "DarkLord"}, {CLASS_SUMMONER, "Summoner"}};
    for (float contentScale : {1.f, 1.5f, 2.f})
    for (int tab = 0; tab < (helper ? 3 : 1); ++tab)
    for (int character : {CLASS_KNIGHT, CLASS_WIZARD, CLASS_ELF, CLASS_DARK_LORD, CLASS_SUMMONER})
    {
        if (!helper && character != CLASS_KNIGHT) continue;
        scenario.contentScale = contentScale;
        scenario.tab = helper ? tab : -1;
        scenario.character = helper ? names.at(character) : "n/a";
        RunScenario(fixture, audit, markup, scenario, character);
    }
}
} // namespace

TEST_CASE("rollout 2 contains event, shop and MU Helper text and records remaining small-scale defects [ui][text-layout]")
{
    const char* directory = std::getenv("MU_RML_TEXT_LAYOUT_CASES");
    REQUIRE_MESSAGE(directory != nullptr, "Run this diagnostic through prepare_rml_text_layout.py or CTest.");
    RmlLayoutFixture fixture;
    RmlTextLayoutAudit audit(directory);
    std::istringstream manifest(UI::Tests::ReadFile(std::filesystem::path(directory) / "manifest.tsv"));
    std::string line;
    int documents = 0;
    while (std::getline(manifest, line))
    {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream row(line);
        RmlTextLayoutAudit::Scenario scenario;
        std::string path;
        std::getline(row, scenario.theme, '\t'); std::getline(row, scenario.window, '\t');
        std::getline(row, scenario.locale, '\t'); std::getline(row, path);
        REQUIRE(!path.empty());
        RunDocument(fixture, audit, scenario, UI::Tests::ReadFile(path));
        ++documents;
    }
    CHECK(documents == ExpectedDocuments);
    audit.Finish(ExpectedScenarios);
    for (const char* window : {"blood_castle_enter", "devil_square_enter"})
        CHECK_MESSAGE(audit.FailureCount(window) == 0, window, " must contain every visible text line.");
    for (const char* window : {"my_shop", "purchase_shop"})
        CHECK_MESSAGE(audit.FailureCount(window) == 0, window, " must contain every visible text line.");
    // Legacy's MU Helper keeps native's positions, its labels clipped to native's room (marquee);
    // what still collides at large text sizes is reported, not required away.
    CHECK_MESSAGE(audit.ThemeFailureCount("modern", "mu_helper_config") == 0,
                  "modern mu_helper_config must contain every visible text line.");
}
