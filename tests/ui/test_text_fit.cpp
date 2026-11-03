#include "stdafx.h"
#include <doctest.h>
#include "RmlLayoutFixture.h"
#include "UI/RmlBridge/RmlNativeTextFit.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementText.h>
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/TextShapingContext.h>

#include <filesystem>
#include <string>

namespace
{
class TextFitFixture : public UI::Tests::RmlLayoutFixture
{
public:
    TextFitFixture() : m_WorkingDirectory(std::filesystem::current_path()), m_Theme(UI::RmlBridge::GetActiveThemeName())
    {
        std::filesystem::current_path(MU_BIN_DIR);
        Configure({1280, 720}, 100);
    }

    ~TextFitFixture()
    {
        UI::RmlBridge::SetActiveThemeName(m_Theme);
        std::filesystem::current_path(m_WorkingDirectory);
    }

    Rml::ElementDocument* LoadText(const char* theme, const char* className, const char* family, const char* value)
    {
        UI::RmlBridge::SetActiveThemeName(theme);
        const std::string markup =
            std::string("<rml><head><link type=\"text/rcss\" href=\"Data/Interface/RmlUi/themes/") + theme +
            "/base.rcss\"/><style>body { margin: 0; } #box { display: block; width: 150px; font-size: 40px; "
            "font-family: '" +
            family +
            "'; } .text-fit, .native-fit { display: block; white-space: nowrap; }</style></head><body>"
            "<div id=\"box\"><span id=\"fit\" class=\"" +
            className + "\">" + value + "</span></div></body></rml>";
        auto* document = LoadMarkup(markup);
        REQUIRE(document->GetElementById("box")->GetOffsetWidth() == doctest::Approx(150.f));
        return document;
    }

private:
    std::filesystem::path m_WorkingDirectory;
    std::string m_Theme;
};

float FontSize(Rml::ElementDocument* document)
{
    return document->GetElementById("fit")->GetComputedValues().font_size();
}

float TextWidth(Rml::Element* element)
{
    const auto& values = element->GetComputedValues();
    REQUIRE(values.font_face_handle() != 0);
    const auto* text = rmlui_dynamic_cast<Rml::ElementText*>(element->GetFirstChild());
    REQUIRE(text != nullptr);
    const Rml::TextShapingContext shaping{values.language(), values.direction(), values.font_kerning(),
                                          values.letter_spacing()};
    return static_cast<float>(
        Rml::GetFontEngineInterface()->GetStringWidth(values.font_face_handle(), text->GetText(), shaping));
}
} // namespace

TEST_CASE("text-fit shrinks by font size in both themes and legacy native-fit stays gated [ui][text-fit]")
{
    TextFitFixture fixture;
    for (const char* theme : {"legacy", "modern"})
    {
        CAPTURE(theme);
        auto* document = fixture.LoadText(theme, "text-fit", "Liberation Sans", "AnExceptionallyLongMapName");
        const float originalSize = FontSize(document);
        UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
        const float fittedSize = FontSize(document);
        CHECK(fittedSize < originalSize);
        CHECK(fittedSize > 0.f);
        CHECK(TextWidth(document->GetElementById("fit")) <=
              document->GetElementById("fit")->GetBox().GetSize(Rml::BoxArea::Content).x + 1.f);
        CHECK(document->GetElementById("fit")->GetLocalProperty(Rml::PropertyId::Transform) == nullptr);

        document = fixture.LoadText(theme, "text-fit", "Liberation Sans", "A");
        UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
        CHECK(FontSize(document) == doctest::Approx(40.f));
        CHECK(document->GetElementById("fit")->GetLocalProperty(Rml::PropertyId::FontSize) == nullptr);
    }

    auto* document = fixture.LoadText("modern", "native-fit", "Liberation Sans", "AnExceptionallyLongMapName");
    CHECK_FALSE(UI::RmlBridge::ThemeUsesNativeTextSize());
    const float originalSize = FontSize(document);
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
    CHECK(FontSize(document) == doctest::Approx(originalSize));
}

TEST_CASE("text-fit invalidates on typography changes and restores parent size for short text [ui][text-fit]")
{
    TextFitFixture fixture;
    auto* document = fixture.LoadText("modern", "text-fit", "Liberation Sans", "MMMMMMMMMMMMMMMMMMMM");
    auto* fit = document->GetElementById("fit");
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
    const float originalFit = FontSize(document);
    REQUIRE(originalFit < 40.f);

    REQUIRE(fit->SetProperty("letter-spacing", "4px"));
    fixture.Refresh();
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
    CHECK(FontSize(document) < originalFit);

    REQUIRE(document->GetElementById("box")->SetProperty("font-family", "'DejaVu Sans'"));
    fixture.Refresh();
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
    CHECK(FontSize(document) < 40.f);
    CHECK(fit->GetComputedValues().font_family() == "dejavu sans");

    auto* text = rmlui_dynamic_cast<Rml::ElementText*>(fit->GetFirstChild());
    REQUIRE(text != nullptr);
    text->SetText("A");
    fixture.Refresh();
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
    CHECK(FontSize(document) == doctest::Approx(40.f));
    CHECK(fit->GetLocalProperty(Rml::PropertyId::FontSize) == nullptr);
}

TEST_CASE("text-fit resolves relative letter spacing at the fitted size without frame feedback [ui][text-fit]")
{
    TextFitFixture fixture;
    for (const char* spacing : {"0.1em", "calc(0.1em + 2px)", "6.5px"})
    {
        CAPTURE(std::string(spacing));
        auto* document = fixture.LoadText("modern", "text-fit", "Liberation Sans", "MMMMMMMMMMMMMMMMMMMM");
        auto* fit = document->GetElementById("fit");
        REQUIRE(fit->SetProperty("letter-spacing", spacing));
        fixture.Refresh();
        UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
        const float fittedSize = FontSize(document);
        REQUIRE(fittedSize < 40.f);
        for (int frame = 0; frame < 8; ++frame)
        {
            CAPTURE(frame);
            fixture.Refresh();
            UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
            CHECK(FontSize(document) == doctest::Approx(fittedSize));
            CHECK(TextWidth(fit) <= fit->GetBox().GetSize(Rml::BoxArea::Content).x + 1.f);
        }
    }
}

TEST_CASE("text-fit measures collapsed spaces and nested inline text as RmlUi lays them out [ui][text-fit]")
{
    TextFitFixture fixture;
    const auto fittedSize = [&fixture](const char* value)
    {
        auto* document = fixture.LoadText("modern", "text-fit", "Liberation Sans", value);
        UI::RmlBridge::FitNativeTextToBoxes(fixture.context);
        return FontSize(document);
    };

    const float plain = fittedSize("MMMMMMMMMM MMMMMMMMMM");
    REQUIRE(plain < 40.f);
    CHECK(fittedSize("  MMMMMMMMMM     MMMMMMMMMM  ") == doctest::Approx(plain));
    CHECK(fittedSize("MMMMMMMMMM <span>MMMMMMMMMM</span>") == doctest::Approx(plain));
}

TEST_CASE("text-fit fits an element created after the first pass [ui][text-fit]")
{
    TextFitFixture fixture;
    auto* document = fixture.LoadText("modern", "text-fit", "Liberation Sans", "A");
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);

    Rml::ElementPtr added = document->CreateElement("span");
    added->SetId("added");
    added->SetClass("text-fit", true);
    added->AppendChild(document->CreateTextNode("AnExceptionallyLongMapName"));
    Rml::Element* addedElement = document->GetElementById("box")->AppendChild(std::move(added));
    fixture.Refresh();
    UI::RmlBridge::FitNativeTextToBoxes(fixture.context);

    CHECK(addedElement->GetComputedValues().font_size() < 40.f);
    CHECK(TextWidth(addedElement) <= addedElement->GetBox().GetSize(Rml::BoxArea::Content).x + 1.f);
}
