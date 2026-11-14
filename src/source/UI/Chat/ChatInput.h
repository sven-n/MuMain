#pragma once

#include <string_view>

// Routes a typed chat line to a slash command, a whisper, or a normal message.
namespace UI::Chat
{
inline constexpr std::wstring_view DanceCommand = L"/dance";

// True for /dance, any casing, optional surrounding whitespace and arguments.
bool IsDanceCommand(std::wstring_view text);
// True for word as a whole word; /word is rejected.
bool ContainsWord(std::wstring_view text, std::wstring_view word);
// True for the translated Dance word as a whole word; rejects /Dance and Dancer.
bool ContainsDanceWord(std::wstring_view text);

void CheckChatText(wchar_t* Text);
}
