#pragma once

#include <string>
#include <string_view>

// The ids the server list gives its own entries, VIP types and colors alike:
// what channels and button color rules name them by (docs/server-list.md).
namespace Data::ServerList
{
// Letters and digits, starting with a lowercase letter: "gold", "goldRed".
bool IsListId(std::string_view id);
// An id made from a name: "Gold Plus" is "goldPlus"; `fallback` when the name
// has no letter to start one with.
std::string MakeListId(std::string_view name, std::string_view fallback);
// The text with A to Z in lowercase: a shade's theme folder name, as the game
// names its themes (UI::RmlBridge::GetActiveThemeName).
std::string ToLowerAscii(std::string_view text);
} // namespace Data::ServerList
