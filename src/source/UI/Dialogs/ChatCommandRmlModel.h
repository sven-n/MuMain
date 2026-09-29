#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One line of the chat command window: left-aligned in its box (centred for the template's
// remove "x"), shrunk to it like the original's RenderText().
struct ChatCommandTextEntry
{
    Rml::String text;
    float left = 0.f;   // reference px in the panel
    float top = 0.f;    // reference px in the panel
    float width = 0.f;  // box width, reference px
    float textPx = 0.f; // physical px
    bool centred = false;
    Rml::String color; // CSS colour of the native text colour
};

// What a click on a ChatCommandHitEntry does (the original's CheckMouseIn() areas).
enum class ChatCommandAction
{
    PickCommand = 1,
    EditValue = 2,
    ToggleFavourite = 3,
    SaveTemplate = 4,
    ExecuteTemplate = 5,
    RemoveTemplate = 6,
};

// A clickable area; a parameter's value area also draws the dark value box.
struct ChatCommandHitEntry
{
    float left = 0.f; // reference px in the panel
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
    int action = 0; // ChatCommandAction
    int index = 0;  // the row, parameter or template
    bool valueBox = false;
};

struct ChatCommandRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    float windowHeight = 0.f;

    ChatCommandTextEntry title;
    std::vector<ChatCommandTextEntry> texts;
    std::vector<ChatCommandHitEntry> hits;

    // The value field of the parameter being edited (the original's CUITextInputBox).
    bool editing = false;
    float editTop = 0.f;

    bool hasLeftButton = false;
    bool hasRightButton = false;
    Rml::String leftText;
    Rml::String rightText;
    // CButton::Render(): 29 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
