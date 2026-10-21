#include "stdafx.h"
#include "Integration/Discord/PresenceText.h"

#include "Core/Text/Utf8.h"
#include "Integration/Discord/WideFormat.h"

#include "I18N/All.h"

namespace
{
using Integration::Discord::FormatWide;
using Integration::Discord::PresenceMode;
using Integration::Discord::PresenceSnapshot;

constexpr const wchar_t* PartSeparator = L" · "; // " · "

std::wstring CharacterLine(const PresenceSnapshot& snapshot)
{
    if (snapshot.masterLevel > 0)
    {
        return FormatWide(I18N::Game::DiscordCharacterMasterLevel, snapshot.level, snapshot.masterLevel,
                          snapshot.className.c_str());
    }
    return FormatWide(I18N::Game::DiscordCharacterLevel, snapshot.level, snapshot.className.c_str());
}

std::wstring LocationText(const PresenceSnapshot& snapshot)
{
    if (!snapshot.inEvent)
    {
        return snapshot.location;
    }

    return FormatWide(I18N::Game::DiscordInEvent, snapshot.location.c_str());
}

std::wstring LocationLine(const PresenceSnapshot& snapshot)
{
    std::wstring line = LocationText(snapshot);
    if (snapshot.partyMembers <= 0)
    {
        return line;
    }

    return line + PartSeparator + FormatWide(I18N::Game::DiscordParty, snapshot.partyMembers, snapshot.partyCapacity);
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
