#pragma once

#include <RmlUi/Core/Types.h>

namespace mu::ui::window
{
// One line of the gatekeeper window's text: the document places it, so only what it says and the
// size the native renderer shrank it to for its box travel through the model.
struct GatemanLine
{
    Rml::String text;
    float textPx = 0.f;

    bool operator==(const GatemanLine&) const = default;
};

// One of the window's two buttons (Enter, Confirm). Where it sits and what a locked one looks
// like are the theme's; whether it is there and whether it is locked are not.
struct GatemanActionButton
{
    Rml::String label;
    bool shown = false;
    bool locked = false;

    bool operator==(const GatemanActionButton&) const = default;
};

struct GatemanRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    float lineHeightPx = 0.f;
    float labelTop = 0.f; // CButton::Render(): 23 / 2 - h / 2 whole units

    // Which page the gatekeeper shows: 0 none, 1 the guild master's, 2 a guild member's,
    // 3 a guest's (CUIGateKeeper::GetType()).
    int page = 0;
    bool isPublic = false;

    GatemanLine title;

    // The guild master's page: the entrance restriction, the public checkbox's label, the fee in
    // force, and the fee it is being set to with its permitted range.
    GatemanLine restriction;
    GatemanLine membersLine1;
    GatemanLine membersLine2;
    GatemanLine membersLine3;
    GatemanLine openToNonMembers;
    GatemanLine entranceFee;
    GatemanLine feeSettingLabel;
    GatemanLine viewFee;
    GatemanLine feeRange;
    GatemanLine feeRangeFor;
    GatemanLine feeIncrement;

    // A guild member's page.
    GatemanLine memberQuestion;

    // A guest's page, when the gate is open: the fee, and whether the hero can afford it.
    GatemanLine guestFee;
    bool guestCanAfford = false;
    GatemanLine guestPayPrompt;
    GatemanLine guestQuestion;

    // A guest's page, when it is not.
    GatemanLine deniedLine1;
    GatemanLine deniedLine2;
    GatemanLine deniedLine3;
    GatemanLine deniedLine4;

    GatemanActionButton confirmButton;
    GatemanActionButton enterButton;

    Rml::String exitTooltip;
};
} // namespace mu::ui::window
