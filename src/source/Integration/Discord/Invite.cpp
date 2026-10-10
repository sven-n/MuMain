#include "Integration/Discord/Invite.h"

#include "Core/Utilities/Log/MuLogger.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_misc.h>

#include <algorithm>
#include <array>
#include <mutex>
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

// What may follow the code: a query or fragment, like the "?event=..." of a
// shared event or the "?utm_source=..." of a copied link. Bounded, and made of
// characters which can't break out of the URL when the system opens it.
constexpr std::size_t MaxSuffixLength = 200;
constexpr std::wstring_view SuffixPunctuation = L"-._~=&%?#+";
constexpr std::wstring_view SuffixStarts = L"?#";

bool IsCodeCharacter(wchar_t character)
{
    return (character >= L'a' && character <= L'z') || (character >= L'A' && character <= L'Z') ||
           (character >= L'0' && character <= L'9') || character == L'-';
}

bool IsSuffixCharacter(wchar_t character)
{
    return IsCodeCharacter(character) || SuffixPunctuation.find(character) != std::wstring_view::npos;
}

bool IsSuffix(std::wstring_view suffix)
{
    if (suffix.size() > MaxSuffixLength)
    {
        return false;
    }
    for (const wchar_t character : suffix)
    {
        if (!IsSuffixCharacter(character))
        {
            return false;
        }
    }
    return true;
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
            const std::wstring_view rest = url.substr(prefix.size());
            const std::size_t suffixStart = std::min(rest.find_first_of(SuffixStarts), rest.size());
            return IsInviteCode(rest.substr(0, suffixStart)) && IsSuffix(rest.substr(suffixStart));
        }
    }
    return false;
}

bool Accept(std::wstring_view url)
{
    if (IsInviteUrl(url))
    {
        return true;
    }
    if (url.empty())
    {
        return false;
    }

    // Said once per link: the options window asks every time it opens.
    static std::mutex reportedMutex;
    static std::wstring reported;
    std::lock_guard lock(reportedMutex);
    if (reported != url)
    {
        reported = url;
        mu::log::Get("discord")->warn("The Discord invite link is not a Discord invite, so it is not offered: {}",
                                      std::string(url.begin(), url.end()));
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
