#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One line of the chat command window's own text. Where it sits and what colour it is are the
// theme's; only what it says and the size the native renderer shrank it to for its box travel.
struct ChatCommandLine
{
    Rml::String text;
    float textPx = 0.f;

    bool operator==(const ChatCommandLine&) const = default;
};

// One command in the list. The favourite marker and the colour that goes with it are the theme's.
struct ChatCommandRow
{
    Rml::String text;
    float textPx = 0.f;
    bool favourite = false;

    bool operator==(const ChatCommandRow&) const = default;
};

// What a click on a row does. The document names these numbers directly in its
// chat_command_hit(action, index) calls.
enum class ChatCommandAction
{
    PickCommand = 1,
    EditValue = 2,
    ToggleFavourite = 3,
    SaveTemplate = 4,
    ExecuteTemplate = 5,
    RemoveTemplate = 6,
};

// One of the selected command's parameters: its label, whether a required value is still missing
// (which is what blocks sending), and the value in its box -- the valid values as a placeholder
// while there is none. The edited one shows the field instead.
struct ChatCommandParameterRow
{
    Rml::String label;
    float labelTextPx = 0.f;
    bool missing = false;
    Rml::String value;
    float valueTextPx = 0.f;
    bool placeholder = false;
    bool edited = false;

    bool operator==(const ChatCommandParameterRow&) const = default;
};

struct ChatCommandRmlModel
{
    // The parameter value being edited. Two-way through data-value, so the typed text lives
    // here rather than in the element's attribute.
    Rml::String editValue;

    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)

    int page = 0; // CChatCommandWindow's PAGE_*
    ChatCommandLine title;
    // Shown in place of a page's rows when it has none.
    ChatCommandLine emptyMessage;

    std::vector<ChatCommandRow> commandRows;
    std::vector<ChatCommandLine> descriptionLines;
    std::vector<ChatCommandLine> templateRows;

    std::vector<ChatCommandParameterRow> parameters;
    ChatCommandLine favouriteAction;
    ChatCommandLine saveAction;

    bool hasLeftButton = false;
    bool hasRightButton = false;
    Rml::String leftText;
    Rml::String rightText;
    // CButton::Render(): 29 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelLinePx = 0.f;
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
