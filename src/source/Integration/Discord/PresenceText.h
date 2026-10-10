// Turns the game's state into the text and images of the Discord presence.
#pragma once

#include "Integration/Discord/Activity.h"
#include "Integration/Discord/PresenceMode.h"
#include "Integration/Discord/PresenceSnapshot.h"

#include <string>

namespace Integration::Discord
{
// Asset keys of the server's Discord application, from config.ini.
struct PresenceImages
{
    std::string largeImageKey;
    std::string smallImageKey;
};

// The activity for an On or HideDetails presence. The start timestamp is the
// caller's to set; it depends on history, not on the snapshot.
[[nodiscard]] Activity DescribePresence(const PresenceSnapshot& snapshot, PresenceMode mode,
                                        const PresenceImages& images);
} // namespace Integration::Discord
