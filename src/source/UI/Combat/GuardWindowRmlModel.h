#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A guild declaration and its registration state.
struct GuardDeclareRow
{
    Rml::String name;
    Rml::String markCount;
    Rml::String state; // acquired the sign, or gave up
    Rml::String order; // the order it registered in
    bool selected = false;

    bool operator==(const GuardDeclareRow&) const = default;
};

// One guild taking part in the siege: which side it joined and whether it is maintaining the
// castle or assisting. A defending guild's row carries the same backdrop a selected one does.
struct GuardSiegeRow
{
    Rml::String name;
    Rml::String side;
    Rml::String involvement;
    bool selected = false;
    bool defending = false;

    bool operator==(const GuardSiegeRow&) const = default;
};

// One of the window's three buttons (Announce, Register, Abandon). Where it sits and what a
// locked one looks like are the theme's; whether it is there and whether it is locked are not.
struct GuardActionButton
{
    Rml::String label;
    bool shown = false;
    bool locked = false;

    bool operator==(const GuardActionButton&) const = default;
};

// One line of the window's own text: the document places it, so only what it says and the size
// the native renderer shrank it to for its box travel through the model.
struct GuardLine
{
    Rml::String text;
    float textPx = 0.f;

    bool operator==(const GuardLine&) const = default;
};

// One of the three newui_guild_tab04 radio tabs (56 x 22): the selected one on its second row,
// its label white, the others (181, 181, 181).
struct GuardTabEntry
{
    Rml::String label;
    bool selected = false;
};

struct GuardWindowRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    float lineHeightPx = 0.f;

    int activeTab = 0; // CGuardWindow::CURR_OPEN_TAB_BUTTON -- which page the document shows.
    std::vector<GuardTabEntry> tabs;

    // The window's own heading, fitted to the original's 160-unit box.
    GuardLine title;
    GuardLine ownerMaster;
    GuardLine ownerGuild;

    // The Status page.
    GuardLine statusStart;
    GuardLine statusEnd;
    GuardLine statusPeriod;
    GuardLine statusExpectedLabel;
    GuardLine statusExpectedTime;
    GuardLine statusNextStage;

    // The Register / Announce page. The original drew the "period has ended" and truce lines in
    // the bold font and every other state's in the normal one; no rule behind that was recoverable,
    // so it stays a flag the theme can ignore.
    GuardLine registerMessage;
    GuardLine registerMessage2;
    GuardLine registerAcquired;
    GuardLine registerRegistered;
    bool registerBold = false;

    // The List page's message, for the states that have no list.
    GuardLine listMessage;

    GuardActionButton proclaimButton;
    GuardActionButton registerButton;
    GuardActionButton giveUpButton;
    // Which guild list the List tab shows: 0 none, 1 the guilds that declared (registration
    // period), 2 the guilds in the siege. Only the siege list has a summary row underneath. Both
    // lists' frames, backdrops, columns and row height are the theme's.
    int listKind = 0;
    std::vector<GuardDeclareRow> declareRows;
    std::vector<GuardSiegeRow> siegeRows;
    // The column headings, and the summary row's own value line.
    Rml::String headerName;
    Rml::String headerMarkCount;
    Rml::String headerState;
    Rml::String headerOrder;
    Rml::String headerSide;
    Rml::String headerInvolvement;
    Rml::String scoreLabel;
    Rml::String scoreValue;

    Rml::String exitTooltip;
};
} // namespace mu::ui::window
