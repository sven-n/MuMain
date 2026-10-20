// The server's Discord invite: checking that a link really is one, and
// opening it in the Discord app or the browser.
//
// The link comes from config.ini today and from the server later (#698), so
// nothing but a Discord invite is ever handed to the system's URL handler.
//
// Builds without ENABLE_DISCORD get the no-ops below.
#pragma once

#include <string_view>

namespace Integration::Discord::Invite
{
#if MU_ENABLE_DISCORD
// Whether `url` is https://discord.gg/<code> or
// https://discord.com/invite/<code> (also discordapp.com), with a code of
// letters, digits and dashes, optionally followed by a query or fragment
// ("?event=...") made of URL-safe characters only.
[[nodiscard]] bool IsInviteUrl(std::wstring_view url);

// Like IsInviteUrl, for a link about to be offered: a non-empty link which
// isn't an invite is logged (once), so a typo in config.ini doesn't just
// look like no invite at all.
[[nodiscard]] bool Accept(std::wstring_view url);

// Opens the invite. False when `url` is not an invite link or the system
// could not open it.
bool Open(std::wstring_view url);
#else
[[nodiscard]] inline bool IsInviteUrl(std::wstring_view)
{
    return false;
}

[[nodiscard]] inline bool Accept(std::wstring_view)
{
    return false;
}

inline bool Open(std::wstring_view)
{
    return false;
}
#endif
} // namespace Integration::Discord::Invite
