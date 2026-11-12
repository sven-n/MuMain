#include "stdafx.h"

#include "LookProfileEdits.h"

#include "Data/GameData/MapData/LightMood.h"

#include <algorithm>
#include <sstream>

namespace Data::Maps
{
namespace
{
// The default's values: no map has this number.
constexpr int NoWorld = -1;
constexpr const char* Source = "Look.json";

std::string FormatNumber(double value)
{
    std::ostringstream text;
    text << value;
    return text.str();
}

std::string FatalMessages(const std::vector<Items::ItemDataIssue>& issues)
{
    std::string text;
    for (const Items::ItemDataIssue& issue : issues)
    {
        if (issue.severity == Items::ItemDataIssueSeverity::Fatal)
        {
            text += (text.empty() ? "" : "; ") + issue.ToString();
        }
    }
    return text;
}

MapLookOverrides& EntryOf(LookProfileData& profile, int world, std::string_view mapName)
{
    if (MapLookOverrides* entry = profile.FindMap(world))
    {
        return *entry;
    }
    profile.maps.push_back({{world}, std::string(mapName), std::nullopt, {}});
    return profile.maps.back();
}

bool NamesNothing(const MapLookOverrides& entry)
{
    return !entry.environment && entry.overrides == LookOverrides{};
}

// Checks the changed profile and, when it passes, makes it the profile.
void Commit(LookProfileData& profile, LookProfileData work, LookEditOutcome& outcome)
{
    std::vector<Items::ItemDataIssue> issues;
    if (!ValidateLookProfile(work, Source, issues))
    {
        outcome.error = FatalMessages(issues);
        outcome.oldValues.clear();
        return;
    }
    outcome.moodChanged = !(MoodsOf(profile) == MoodsOf(work));
    profile = std::move(work);
    outcome.applied = true;
}
} // namespace

LookEditOutcome ApplyLookEdits(LookProfileData& profile, std::span<const LookValueEdit> edits, std::optional<int> world,
                               std::string_view mapName)
{
    LookEditOutcome outcome;
    LookProfileData work = profile;
    const LookValues before = profile.ForWorld(world.value_or(NoWorld));
    for (const LookValueEdit& edit : edits)
    {
        const LookValueInfo& info = Info(edit.value);
        if (!IsInRange(info, edit.number))
        {
            outcome.error = std::string(info.name) + ": " + FormatNumber(edit.number) + " is outside " +
                            FormatNumber(info.min) + " to " + FormatNumber(info.max);
            outcome.oldValues.clear();
            return outcome;
        }
        outcome.oldValues.push_back(before[edit.value]);
        LookOverrides& target = world ? EntryOf(work, *world, mapName).overrides : work.defaultOverrides;
        target[edit.value] = edit.number;
    }
    if (const MapLookOverrides* entry = world ? work.FindMap(*world) : nullptr)
    {
        outcome.entryMaps = entry->maps;
    }
    Commit(profile, std::move(work), outcome);
    return outcome;
}

LookEditOutcome ResetLookValue(LookProfileData& profile, const LookProfileData& loaded, LookValue value, int world)
{
    LookEditOutcome outcome;
    LookProfileData work = profile;
    outcome.oldValues.push_back(profile.ForWorld(world)[value]);
    work.defaultOverrides[value] = loaded.defaultOverrides[value];

    const MapLookOverrides* loadedEntry = loaded.FindMap(world);
    if (loadedEntry && loadedEntry->overrides[value])
    {
        MapLookOverrides& entry = EntryOf(work, world, loadedEntry->name);
        entry.overrides[value] = loadedEntry->overrides[value];
        outcome.entryMaps = entry.maps;
    }
    else if (MapLookOverrides* entry = work.FindMap(world))
    {
        entry->overrides[value].reset();
        outcome.entryMaps = entry->maps;
        if (!loadedEntry && NamesNothing(*entry))
        {
            work.maps.erase(work.maps.begin() + (entry - work.maps.data()));
        }
    }
    Commit(profile, std::move(work), outcome);
    return outcome;
}
} // namespace Data::Maps
