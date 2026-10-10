// What the player's Discord profile shows while the client runs: the two text
// lines, the images and the "elapsed" timer. UTF-8 throughout, as Discord
// takes it.
#pragma once

#include <cstdint>
#include <string>

namespace Integration::Discord
{
struct Activity
{
    std::string details;
    std::string state;

    // Asset keys of the images uploaded to the server's Discord application,
    // and the tooltip shown when hovering them. An empty key shows no image.
    std::string largeImageKey;
    std::string largeImageText;
    std::string smallImageKey;
    std::string smallImageText;

    // Unix time in seconds the "elapsed" timer counts from; 0 shows none.
    std::int64_t startTimestamp = 0;

    bool operator==(const Activity&) const = default;
};
} // namespace Integration::Discord
