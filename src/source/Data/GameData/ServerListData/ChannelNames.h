#pragma once

#include "Core/Globals/_define.h"
#include "Data/GameData/Common/LocalizedString.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace Data::ServerList
{
// The places that show a channel's name; each has a template of its own.
enum class ChannelNamePlace : std::uint8_t
{
    // The channel's button on the login screen.
    Button,
    // The login window's title.
    Login,
    // The character info window's title.
    CharacterInfo,
    // What the chat message about a saved screenshot ends with.
    Screenshot,
};

inline constexpr int ChannelNamePlaceCount = 4;

// A template, or the PvP and Non-PvP text, in UTF-16 units.
inline constexpr int MaxChannelNameTextLength = MAX_TEXT_LENGTH - 1;

// The texts that name a channel, each with its translations.
struct ChannelNames
{
    // {pvp} on a channel with PvP on, and with PvP off.
    LocalizedString pvp;
    LocalizedString nonPvp;
    // By ChannelNamePlace.
    std::array<LocalizedString, ChannelNamePlaceCount> templates;

    LocalizedString& Template(ChannelNamePlace place)
    {
        return templates[static_cast<size_t>(place)];
    }
    const LocalizedString& Template(ChannelNamePlace place) const
    {
        return templates[static_cast<size_t>(place)];
    }

    bool operator==(const ChannelNames&) const = default;
};

// The built-in English texts: what the texts a file leaves out are.
ChannelNames GetDefaultChannelNames();
} // namespace Data::ServerList
