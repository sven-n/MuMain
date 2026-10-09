#include "stdafx.h"
#include "Integration/Discord/PresenceText.h"

#include "Core/Text/Utf8.h"

#include "I18N/All.h"

namespace
{
using Integration::Discord::PresenceMode;
using Integration::Discord::PresenceSnapshot;

constexpr std::size_t LineBufferLength = 128;
constexpr const wchar_t* PartSeparator = L" · "; // " · "

std::wstring CharacterLine(const PresenceSnapshot& snapshot)
{
    wchar_t line[LineBufferLength]{};
    if (snapshot.masterLevel > 0)
    {
        mu_swprintf_s(line, LineBufferLength, I18N::Game::DiscordCharacterMasterLevel, snapshot.level,
                      snapshot.masterLevel, snapshot.className.c_str());
    }
    else
    {
        mu_swprintf_s(line, LineBufferLength, I18N::Game::DiscordCharacterLevel, snapshot.level,
                      snapshot.className.c_str());
    }
    return line;
}

std::wstring LocationText(const PresenceSnapshot& snapshot)
{
    if (!snapshot.inEvent)
    {
        return snapshot.location;
    }

    wchar_t text[LineBufferLength]{};
    mu_swprintf_s(text, LineBufferLength, I18N::Game::DiscordInEvent, snapshot.location.c_str());
    return text;
}

std::wstring LocationLine(const PresenceSnapshot& snapshot)
{
    std::wstring line = LocationText(snapshot);
    if (snapshot.partyMembers <= 0)
    {
        return line;
    }

    wchar_t party[LineBufferLength]{};
    mu_swprintf_s(party, LineBufferLength, I18N::Game::DiscordParty, snapshot.partyMembers, snapshot.partyCapacity);
    return line + PartSeparator + party;
}

void DescribeWorld(Integration::Discord::Activity& activity, const PresenceSnapshot& snapshot, PresenceMode mode)
{
    const bool showCharacter = mode == PresenceMode::On;
    activity.details = Core::Text::ToUtf8(showCharacter ? CharacterLine(snapshot).c_str() : I18N::Game::DiscordInGame);
    activity.state = Core::Text::ToUtf8(LocationLine(snapshot).c_str());
    activity.largeImageText = Core::Text::ToUtf8(snapshot.location.c_str());
    if (showCharacter)
    {
        activity.smallImageText = Core::Text::ToUtf8(snapshot.className.c_str());
    }
}
} // namespace

namespace Integration::Discord
{
Activity DescribePresence(const PresenceSnapshot& snapshot, PresenceMode mode, const PresenceImages& images)
{
    Activity activity;
    activity.largeImageKey = images.largeImageKey;
    activity.smallImageKey = images.smallImageKey;

    switch (snapshot.scene)
    {
    case PresenceSnapshot::Scene::Login:
        activity.details = Core::Text::ToUtf8(I18N::Game::DiscordInLogin);
        break;
    case PresenceSnapshot::Scene::CharacterSelect:
        activity.details = Core::Text::ToUtf8(I18N::Game::DiscordInCharacterSelect);
        break;
    case PresenceSnapshot::Scene::World:
        DescribeWorld(activity, snapshot, mode);
        break;
    }
    return activity;
}
} // namespace Integration::Discord
