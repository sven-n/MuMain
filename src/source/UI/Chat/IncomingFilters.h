#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>

// A place to change or hide a message the server sent before the player sees it: for servers which put their own data
// into chat lines, for translating them, or for a mute list.
namespace UI::Chat
{
enum class IncomingKind
{
    Banner,  // The notice in the middle of the screen, and the scrolling ones.
    System,  // The server's line in the message log.
    Guild,   // The notice of the guild.
    Whisper, // A whisper of another player.
};

struct IncomingMessage
{
    IncomingKind kind;
    std::wstring_view sender; // Empty for what the server says itself.
    std::wstring text;
};

// May change the text. Returns false to hide the message.
using IncomingFilter = std::function<bool(IncomingMessage&)>;

// Filters run in the order they were added, each on the text the former one left. A filter must not add or remove
// filters while it runs. Returns the number to remove it with.
int AddIncomingFilter(IncomingFilter filter);
void RemoveIncomingFilter(int id);

// Runs the filters. False: the message is hidden, and the filters after the one which hid it did not run.
bool ApplyIncomingFilters(IncomingMessage& message);

// The same on the buffer a message was received into; a longer new text is cut to fit.
template <std::size_t Size> bool FilterIncoming(IncomingKind kind, std::wstring_view sender, wchar_t (&text)[Size])
{
    IncomingMessage message{kind, sender, text};
    if (!ApplyIncomingFilters(message))
    {
        return false;
    }

    const std::size_t length = (std::min)(message.text.size(), Size - 1);
    std::copy_n(message.text.data(), length, text);
    text[length] = L'\0';
    return true;
}
} // namespace UI::Chat
