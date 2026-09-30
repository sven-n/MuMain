#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderText() of the guardsman window: its box and alignment, font and colour, its size
// shrunk to the box like the original's.
struct GuardTextEntry
{
    Rml::String text;
    float left = 0.f; // reference px in the panel
    float top = 0.f;
    float width = 0.f; // 0 = no box
    float textPx = 0.f;
    int align = 0; // 0 left, 1 centred, 2 right edge at left + width
    bool bold = false;
    Rml::String color;
};

// A RenderColor() box: the lists' backdrops and selected lines.
struct GuardBoxEntry
{
    float left = 0.f;
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
    Rml::String color;
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
    float buttonLabelTop = 0.f; // CButton::Render(): 23 / 2 - h / 2 whole units
    float tabLabelTop = 0.f;    // CRadioButton::Render(): 22 / 2 - h / 2 whole units

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
    // The List tab's frame: 0 none, 1 the declared guilds (registration), 2 the siege guilds.
    // Whether the guild list is on screen, and whether it has its own summary row underneath.
    // Its frame is the theme's to draw; these two say what there is to frame.
    bool listShown = false;
    bool listHasFooter = false;
    bool scrollShown = false;
    float scrollTop = 0.f; // the track, reference px
    float scrollHeight = 0.f;
    float thumbTop = 0.f;
    bool thumbDragged = false;
    std::vector<GuardBoxEntry> boxes;
    std::vector<GuardTextEntry> texts;
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
