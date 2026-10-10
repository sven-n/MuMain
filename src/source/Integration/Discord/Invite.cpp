#include "Integration/Discord/Invite.h"

#include "Core/Utilities/Log/MuLogger.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_misc.h>

#include <array>
#include <string>

namespace
{
// Where an invite code starts, for each form of invite link Discord hands out.
constexpr std::array<std::wstring_view, 4> InvitePrefixes = {
    L"https://discord.gg/",
    L"https://discord.com/invite/",
    L"https://www.discord.com/invite/",
    L"https://discordapp.com/invite/",
};

// Discord's codes are short (vanity URLs included); anything longer is not
// one of them.
constexpr std::size_t MinCodeLength = 2;
constexpr std::size_t MaxCodeLength = 32;

bool IsCodeCharacter(wchar_t character)
{
    return (character >= L'a' && character <= L'z') || (character >= L'A' && character <= L'Z') ||
           (character >= L'0' && character <= L'9') || character == L'-';
}

bool IsInviteCode(std::wstring_view code)
{
    if (code.size() < MinCodeLength || code.size() > MaxCodeLength)
    {
        return false;
    }
    for (const wchar_t character : code)
    {
        if (!IsCodeCharacter(character))
        {
            return false;
        }
    }
    return true;
}
} // namespace

namespace Integration::Discord::Invite
{
bool IsInviteUrl(std::wstring_view url)
{
    for (const std::wstring_view prefix : InvitePrefixes)
    {
        if (url.starts_with(prefix))
        {
            return IsInviteCode(url.substr(prefix.size()));
        }
    }
    return false;
}

bool Open(std::wstring_view url)
{
    if (!IsInviteUrl(url))
    {
        return false;
    }

    // Checked above: the link is plain ASCII.
    const std::string narrowUrl(url.begin(), url.end());
    if (!SDL_OpenURL(narrowUrl.c_str()))
    {
        mu::log::Get("discord")->warn("Could not open the Discord invite {}: {}", narrowUrl, SDL_GetError());
        return false;
    }
    return true;
}
} // namespace Integration::Discord::Invite
