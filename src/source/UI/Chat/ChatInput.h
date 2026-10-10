#pragma once

// Routes a typed chat line to a slash command, a whisper, or a normal message.
namespace UI::Chat
{
    void CheckChatText(wchar_t* Text);
}
