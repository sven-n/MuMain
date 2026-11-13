#pragma once

#include "Data/GameData/MapData/LookProfile.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The look profile's edit operations: every change to a LookProfileData goes
// through them, so each one is checked (ValidateLookProfile) before it lands.
// A refused change leaves the profile as it was.
namespace Data::Maps
{
struct LookValueEdit
{
    LookValue value;
    double number = 0.0;
};

struct LookEditOutcome
{
    bool applied = false;
    std::string error;
    // The value each edit replaced, where it applies (the world, or the default).
    std::vector<double> oldValues;
    // Whether the mood of any world changed (MoodsOf): the static map light then rebuilds.
    bool moodChanged = false;
    // The maps of the entry a map edit changed, which all get it.
    std::vector<int> entryMaps;
};

// Sets every value of `edits`, as one change: on the default, or on the entry
// that has `world` (a new one with only that map, named `mapName`, when none
// does). Each value must be in its range, and the profile after all of them
// must pass ValidateLookProfile.
[[nodiscard]] LookEditOutcome ApplyLookEdits(LookProfileData& profile, std::span<const LookValueEdit> edits,
                                             std::optional<int> world, std::string_view mapName = {});

// Puts `value` back as `loaded` has it, on the default and on the entry of
// `world`: its loaded value, or none when the loaded entry named none. An entry
// the edits added and that names nothing any more goes.
[[nodiscard]] LookEditOutcome ResetLookValue(LookProfileData& profile, const LookProfileData& loaded, LookValue value,
                                             int world);
} // namespace Data::Maps
