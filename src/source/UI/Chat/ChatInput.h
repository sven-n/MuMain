#pragma once

// Chat input handling: takes a line the player typed and routes it - a slash
// command, a whisper, or a normal message. Extracted from ZzzInterface.cpp.
namespace UI::Chat
{
    void CheckChatText(wchar_t* Text);
}
