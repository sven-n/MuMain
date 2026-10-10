#pragma once

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Types.h>
#include <vector>

namespace UI::Social
{
struct ChatRoomModel
{
    // `type` is CUISimpleChatListBox::RenderDataLine()'s own colour index, carried as the line's
    // class so each theme owns the palette rather than a colour being pushed from C++.
    struct Line
    {
        Rml::String speaker, text;
        int type = 3;
    };
    struct Pal
    {
        Rml::String name;
        int number = 0;
    };

    std::vector<Line> lines;
    std::vector<Pal> pals;
    std::vector<Pal> invitePals;
    Rml::String title, draft, inviteLabel, selectedInvite;
    Rml::String inviteButtonLabel;
    // window_shell's own contract: a titled panel placed at a final device-pixel top-left.
    float rootX = 0, rootY = 0;
    // The participant column only appears once the room is more than a pair, or while inviting,
    // exactly as CUIChatWindow::RenderSub() gated m_PalListBox.
    bool showInvite = false, showPals = false, locked = false;
    bool hasTitle = true, positioned = true, maximized = false;

    void Bind(Rml::DataModelConstructor& constructor);
};
} // namespace UI::Social
