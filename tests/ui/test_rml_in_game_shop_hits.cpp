// Tests that the cash shop's display-only boxes never take the clicks meant for its buttons.

#include "stdafx.h"
#include <doctest.h>

#include "RmlLayoutFixture.h"
#include "UI/RmlBridge/RmlElementBox.h"

#include <RmlUi/Core.h>

#include <filesystem>
#include <string>

namespace
{
// Classic builds on legacy (its theme.ini's Base): the game gives it legacy's base.rcss.
std::string ShopMarkup(const std::string& theme)
{
    std::string markup = UI::Tests::AssetMarkup(theme, "in_game_shop");
    if (theme == "classic")
    {
        const std::string root = std::filesystem::path(MU_RMLUI_DIR).generic_string();
        const std::string shared = "href=\"" + root + "/base.rcss\"";
        const size_t link = markup.find(shared);
        REQUIRE(link != std::string::npos);
        markup.replace(link, shared.size(), "href=\"" + root + "/themes/legacy/base.rcss\"");
    }
    return markup;
}

// The test markup has one of each data-for row; the shop shows several.
void Repeat(Rml::ElementDocument* document, const char* selector, int count)
{
    Rml::Element* row = document->QuerySelector(selector);
    REQUIRE(row != nullptr);
    for (int copy = 1; copy < count; ++copy)
    {
        row->GetParentNode()->AppendChild(row->Clone());
    }
}

bool IsWithin(Rml::Element* hit, Rml::Element* button)
{
    for (; hit != nullptr; hit = hit->GetParentNode())
    {
        if (hit == button)
        {
            return true;
        }
    }
    return false;
}

// The rows of the button's drawn box, at its centre column, that some other box takes.
std::string RowsTakenFromButton(Rml::Context* context, Rml::Element* button)
{
    Rml::Vector2f offset, size;
    REQUIRE(UI::RmlBridge::DrawnBox(*button, Rml::BoxArea::Border, offset, size));
    const float x = offset.x + size.x * 0.5f;
    std::string taken;
    for (float y = offset.y + 0.5f; y < offset.y + size.y; y += 1.f)
    {
        Rml::Element* hit = context->GetElementAtPoint({x, y});
        if (!IsWithin(hit, button))
        {
            taken += " y" + std::to_string(static_cast<int>(y)) + ":" + (hit ? hit->GetAddress(false, false) : "none");
        }
    }
    return taken;
}
} // namespace

TEST_CASE("every cash shop button takes the clicks over its whole box once the shop sits in a workspace slot "
          "[ui][styling]")
{
    // A slot-placed #panel takes pointer events (base.rcss), and every box under it without its own
    // `pointer-events: none` inherits them, so a display-only box drawn over a button took its clicks.
    UI::Tests::RmlLayoutFixture fixture;
    fixture.Configure({1024, 768}, 100);
    for (const std::string theme : {"legacy", "classic", "modern"})
    {
        CAPTURE(theme);
        Rml::ElementDocument* document = fixture.LoadMarkup(ShopMarkup(theme));
        Repeat(document, ".igs-zone", 4);
        Repeat(document, ".igs-category", 8);
        Repeat(document, ".igs-package", 9);
        Repeat(document, ".igs-wallet-row", 3);
        Repeat(document, ".igs-storage-tab", 2);
        document->GetElementById("panel")->SetClass("slot-placed", true);
        fixture.Refresh();

        Rml::ElementList buttons;
        document->QuerySelectorAll(buttons, ".igs-zone, .igs-category, .igs-buy, .igs-page-btn, .igs-icon-btn, "
                                            ".igs-storage-tab, .igs-storage-page-btn, #igs_use, #igs_close");
        REQUIRE(buttons.size() >= 30);
        for (Rml::Element* button : buttons)
        {
            CAPTURE(button->GetAddress(false, false));
            CHECK(RowsTakenFromButton(fixture.context, button) == "");
        }
        document->Close();
    }
}
