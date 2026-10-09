// The part of the game state the Discord presence is made of, read once per
// refresh so the text can be built without touching game globals.
#pragma once

#include <string>

namespace Integration::Discord
{
struct PresenceSnapshot
{
    enum class Scene
    {
        Login,
        CharacterSelect,
        World,
    };

    Scene scene = Scene::Login;

    // World only.
    std::wstring className;
    int level = 0;
    int masterLevel = 0; // 0 for a character without master levels

    // The map name, or the event and its level ("Blood Castle 5").
    std::wstring location;
    bool inEvent = false;

    int partyMembers = 0; // 0 outside a party
    int partyCapacity = 0;
};
} // namespace Integration::Discord
