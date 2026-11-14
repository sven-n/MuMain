#pragma once

#include "Data/GameData/MapData/LookProfile.h"

#include <cstddef>
#include <cstdint>
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

// Where a change lands: the default, an environment (its index in
// `environments`), or the entry that has a map.
struct LookScope
{
    enum class Kind : std::uint8_t
    {
        Default,
        Environment,
        Map,
    };

    Kind kind = Kind::Default;
    std::size_t environment = 0;
    int world = 0;

    [[nodiscard]] static LookScope Default()
    {
        return {};
    }
    [[nodiscard]] static LookScope Environment(std::size_t index)
    {
        return {Kind::Environment, index, 0};
    }
    [[nodiscard]] static LookScope Map(int world)
    {
        return {Kind::Map, 0, world};
    }

    bool operator==(const LookScope&) const = default;
};

struct LookEditOutcome
{
    bool applied = false;
    std::string error;
    // The value each edit replaced, where it applies (the world, the environment or the default).
    std::vector<double> oldValues;
    // Whether the mood of any world changed (MoodsOf): the static map light then rebuilds.
    bool moodChanged = false;
    // The maps the change reaches: the maps of the entry a map change changed, or the maps
    // using the environment an environment change changed. Empty for the default.
    std::vector<int> entryMaps;
};

// Sets every value of `edits`, as one change, on `scope`. A map without an
// entry gets a new one with only that map, named `mapName`. Each value must be
// in its range, and the profile after all of them must pass ValidateLookProfile.
[[nodiscard]] LookEditOutcome ApplyLookEdits(LookProfileData& profile, std::span<const LookValueEdit> edits,
                                             const LookScope& scope, std::string_view mapName = {});

// The same on the default (`world` empty) or on the entry that has `world`.
[[nodiscard]] LookEditOutcome ApplyLookEdits(LookProfileData& profile, std::span<const LookValueEdit> edits,
                                             std::optional<int> world, std::string_view mapName = {});

// Removes `value` from `scope`, which then takes it from the layer below. An
// entry that names nothing any more goes when it has one map; one shared by
// variants stays, keeping them together.
[[nodiscard]] LookEditOutcome ClearLookValue(LookProfileData& profile, LookValue value, const LookScope& scope);

// The maps whose entry uses environment `environment`, in ascending order.
[[nodiscard]] std::vector<int> MapsUsingEnvironment(const LookProfileData& profile, std::size_t environment);

// Makes the entry that has `world` use `environment` (none when empty); a map
// without an entry gets a new one with only that map, named `mapName`. An
// entry of one map left naming nothing goes.
[[nodiscard]] LookEditOutcome SetMapEnvironment(LookProfileData& profile, int world,
                                                std::optional<std::size_t> environment, std::string_view mapName = {});

// Adds an environment named `name` with no values, at the end. The name must
// be one no other environment has.
[[nodiscard]] LookEditOutcome AddEnvironment(LookProfileData& profile, std::string_view name);

// Renames environment `environment`; the maps using it keep it.
[[nodiscard]] LookEditOutcome RenameEnvironment(LookProfileData& profile, std::size_t environment,
                                                std::string_view name);

// Removes environment `environment`, refused while a map uses it (the error
// names those maps).
[[nodiscard]] LookEditOutcome RemoveEnvironment(LookProfileData& profile, std::size_t environment);

// Makes `replacement` the profile (an undo or redo of the edits above).
[[nodiscard]] LookEditOutcome ReplaceLookProfile(LookProfileData& profile, const LookProfileData& replacement);

// Puts `value` back as `loaded` has it, on the default and on the entry of
// `world`: its loaded value, or none when the loaded entry named none. An entry
// the edits added and that names nothing any more goes.
[[nodiscard]] LookEditOutcome ResetLookValue(LookProfileData& profile, const LookProfileData& loaded, LookValue value,
                                             int world);
} // namespace Data::Maps
