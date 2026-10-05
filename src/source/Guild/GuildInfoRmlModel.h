#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One line of the guild window's own text. Where it sits, what colour it is and how it is aligned
// are the theme's; only what it says and the size the native renderer shrank it to for its box
// travel through the model.
struct GuildLine
{
    Rml::String text;
    float textPx = 0.f;

    bool operator==(const GuildLine&) const = default;
};

// Announcement lines in reading order.
struct GuildNoticeRow
{
    Rml::String text;

    bool operator==(const GuildNoticeRow&) const = default;
};

// One guild member: the name, the office they hold and which server they are on. A member holding
// an office carries the row backdrop a selected line does, which is what `officer` says.
struct GuildMemberRow
{
    Rml::String name;
    Rml::String role;
    float roleTextPx = 0.f; // the office is centred on 70 units and shrunk to them
    Rml::String server;
    bool selected = false;
    bool officer = false;

    bool operator==(const GuildMemberRow&) const = default;
};

// One allied guild: its mark, its name and how many members it has.
struct GuildUnionRow
{
    Rml::String name;
    Rml::String memberCount;
    float countTextPx = 0.f; // ends at its box's right edge, shrunk to 60 units
    std::vector<Rml::String> markCells; // 64 CSS colours, row by row
    bool selected = false;

    bool operator==(const GuildUnionRow&) const = default;
};

// One of the window's buttons. Where it sits is the theme's; whether it is there is not.
struct GuildActionButton
{
    Rml::String label;
    bool shown = false;

    bool operator==(const GuildActionButton&) const = default;
};

struct GuildInfoRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float panelWidth = 190.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)

    bool noGuild = true;
    int tab = 1; // GuildConstants::GuildTab
    bool unionShown = false;
    std::vector<Rml::String> markCells; // the hero's guild mark (Guild tab)

    // The no-guild hint.
    GuildLine hintTitle;
    GuildLine hintLine1;
    GuildLine hintLine2;
    GuildLine hintLine3;

    // The header every tab shares.
    GuildLine title;
    GuildLine guildName;
    GuildLine tabInfo;
    GuildLine tabMembers;
    GuildLine tabUnion;

    // The Guild tab's own figures.
    GuildLine noticeLabel;
    GuildLine created;
    GuildLine score;
    GuildLine memberCount;
    GuildLine rival;

    // The Members and Alliance tabs' column headings.
    GuildLine headerName;
    GuildLine headerPosition;
    GuildLine headerServer;
    GuildLine headerUnionName;
    GuildLine headerUnionMembers;

    std::vector<GuildNoticeRow> noticeRows;
    std::vector<GuildMemberRow> memberRows;
    std::vector<GuildUnionRow> unionRows;
    // Render_Guild_Info()'s explanation, shown while the hero is in no alliance.
    std::vector<GuildLine> allianceLines;

    GuildActionButton guildOutButton;
    GuildActionButton getPositionButton;
    GuildActionButton freePositionButton;
    GuildActionButton getOutButton;
    GuildActionButton unionCreateButton;
    GuildActionButton unionOutButton;

    Rml::String exitTooltip;
    // CButton::Render(): 29 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelLinePx = 0.f;
};
} // namespace mu::ui::window
