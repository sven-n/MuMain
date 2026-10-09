// How much of the player's state the Discord presence shows; the setting the
// options window offers and config.ini stores as [Discord] Presence.
#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace Integration::Discord
{
enum class PresenceMode : std::uint8_t
{
    On,          // character, level, location and party
    HideDetails, // location and party; no character, class or level
    Off,         // no presence at all
};

inline constexpr std::array<PresenceMode, 3> PresenceModes = {PresenceMode::On, PresenceMode::HideDetails,
                                                              PresenceMode::Off};

inline constexpr PresenceMode DefaultPresenceMode = PresenceMode::On;

// The value written to config.ini.
[[nodiscard]] constexpr const wchar_t* PresenceModeName(PresenceMode mode)
{
    switch (mode)
    {
    case PresenceMode::HideDetails:
        return L"HideDetails";
    case PresenceMode::Off:
        return L"Off";
    case PresenceMode::On:
    default:
        return L"On";
    }
}

// An unknown value (a typo in config.ini) falls back to the default.
[[nodiscard]] constexpr PresenceMode ParsePresenceMode(std::wstring_view name)
{
    for (const PresenceMode mode : PresenceModes)
    {
        if (name == PresenceModeName(mode))
        {
            return mode;
        }
    }
    return DefaultPresenceMode;
}
} // namespace Integration::Discord
