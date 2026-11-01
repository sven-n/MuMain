#pragma once

#include "Data/GameData/ServerListData/LoadBarColors.h"
#include "Data/GameData/ServerListData/LoadState.h"
#include "Data/GameData/ServerListData/LoadText.h"

namespace Data::ServerList
{
// The load section of the file: the {load} texts and the colors of the load bar.
struct LoadSettings
{
    LoadBarColors bar;
    LoadText full;
    LoadText overloaded;

    // The text of a state; nullptr for Open, which has none.
    const LoadText* Text(LoadState state) const;
    LoadText* Text(LoadState state);

    bool operator==(const LoadSettings&) const = default;
};

// The built-in settings: "(Full)" and "Overloaded" in English, in the button's color, and the
// theme's own bar.
LoadSettings GetDefaultLoadSettings();
} // namespace Data::ServerList
