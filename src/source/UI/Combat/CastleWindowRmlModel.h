#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One line of the Senatus's text: the document places it, so only what it says and the size the
// native renderer shrank it to for its box travel through the model.
struct CastleLine
{
    Rml::String text;
    float textPx = 0.f;

    bool operator==(const CastleLine&) const = default;
};

// One of the page's buttons (Buy, Repair, Improve, Apply, Withdraw). Where it sits and what a
// locked one looks like are the theme's; whether it is there and whether it is locked are not.
struct CastleActionButton
{
    Rml::String label;
    bool shown = false;
    bool locked = false;

    bool operator==(const CastleActionButton&) const = default;
};

// One gate or statue standing on the castle map. RCSS places its slot on the map art; the
// model carries whether it stands, whether it is picked, and how far its bars are filled.
// A gate has two bars, a statue three.
struct CastleMapItem
{
    bool statue = false;
    bool live = false;
    bool current = false;
    float hpWidth = 0.f;     // the HP level reached
    float hpFillWidth = 0.f; // how much of that the NPC still has
    float defenseWidth = 0.f;
    float recoverWidth = 0.f;

    bool operator==(const CastleMapItem&) const = default;
};

// One of the four newui_guild_tab04 radio tabs (the left 40 x 22 of the sprite): the selected
// one on its second row, its label white, the others (181, 181, 181).
struct CastleTabEntry
{
    Rml::String label;
    float textPx = 0.f; // shrunk towards the tab's 40 px like the native RenderText()
    bool selected = false;

    bool operator==(const CastleTabEntry&) const = default;
};

struct CastleWindowRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    float lineHeightPx = 0.f;
    float buttonLabelTop = 0.f; // CButton::Render(): 23 / 2 - h / 2 whole units
    float tabLabelTop = 0.f;    // CRadioButton::Render(): 22 / 2 - h / 2 whole units

    int activeTab = 0; // CCastleWindow::CURR_OPEN_TAB_BUTTON -- which page the document shows.
    std::vector<CastleTabEntry> tabs;
    CastleLine title;

    // The castle gate and guardian statue pages, which share one layout: the map with its items,
    // the picked one's figures, and what the next upgrade would add. A statue has a recovery row
    // the gate has not, and nothing below the map exists until the picked one stands.
    bool statuePage = false;
    bool itemLive = false;
    std::vector<CastleMapItem> mapItems;
    CastleLine mapTitle;
    CastleLine improveTitle;
    CastleLine statHp;
    CastleLine statDefense;
    CastleLine statRecover;
    CastleLine nextHp;
    CastleLine nextDefense;
    CastleLine nextRecover;
    CastleActionButton buyButton;
    CastleActionButton repairButton;
    CastleActionButton hpButton;
    CastleActionButton defenseButton;
    CastleActionButton recoverButton;

    // The tax page: the two rates, the rules the lord has to work within, and the castle's purse.
    CastleLine taxTitle;
    CastleLine chaosRate;
    CastleLine storeRate;
    CastleLine note1;
    CastleLine note2;
    CastleLine note3;
    CastleLine rule1;
    CastleLine rule2;
    CastleLine rule3;
    CastleLine rule4;
    CastleLine rule5;
    CastleLine rule6;
    CastleLine zenLabel;
    CastleLine castleMoney;
    CastleLine footer1;
    CastleLine footer2;
    CastleLine footer3;
    CastleActionButton applyButton;
    CastleActionButton withdrawButton;

    Rml::String exitTooltip;
};
} // namespace mu::ui::window
