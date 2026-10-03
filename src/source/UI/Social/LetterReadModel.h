#pragma once

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Types.h>
#include <vector>

namespace UI::Social
{
struct LetterReadModel
{
    struct Line
    {
        Rml::String text;
    };

    // The body, split on newlines as CUILetterReadWindow::SetLetter() split it; RmlUi wraps the
    // rest, where the native box did its own CutText3 pass.
    std::vector<Line> lines;
    Rml::String title, header;
    Rml::String replyLabel, deleteLabel, closeLabel, prevLabel, nextLabel;
    // window_shell's own contract: a titled panel placed at a final device-pixel top-left.
    float rootX = 0, rootY = 0, workspaceHeight = 0;
    bool hasTitle = true, positioned = true, maximized = false;

    void Bind(Rml::DataModelConstructor& constructor);
};
} // namespace UI::Social
