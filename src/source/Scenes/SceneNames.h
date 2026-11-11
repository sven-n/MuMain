#pragma once

// SceneNames.h - The names each scene goes by, kept in one table.

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"

namespace Scenes
{
struct SceneNames
{
    const char* id;          // control socket protocol: "login", "character_list", "world", ...
    const char* displayName; // $details overlay: "Login", "Character select", "In game", ...
};

// Names of that scene; "unknown" / "Unknown" for a value outside EGameScene.
[[nodiscard]] SceneNames NamesOf(EGameScene scene);
} // namespace Scenes
