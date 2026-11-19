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

// An entry of one map that names nothing any more goes; one shared by variants stays, keeping them together.
void DropIfEmpty(LookProfileData& profile, int world)
{
    const MapLookOverrides* entry = profile.FindMap(world);
    if (entry && entry->maps.size() == 1 && NamesNothing(*entry))
    {
        profile.maps.erase(profile.maps.begin() + (entry - profile.maps.data()));
    }
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

std::string NoEnvironment(std::size_t environment)
{
    return "there is no environment " + std::to_string(environment);
}

// The values `scope` resolves to, and the maps it reaches.
LookValues ValuesOf(const LookProfileData& profile, const LookScope& scope)
{
    switch (scope.kind)
    {
    case LookScope::Kind::Environment:
        return profile.ForEnvironment(scope.environment);
    case LookScope::Kind::Map:
        return profile.ForWorld(scope.world);
    case LookScope::Kind::Default:
        break;
    }
    return profile.ForWorld(NoWorld);
}

std::vector<int> MapsReached(const LookProfileData& profile, const LookScope& scope)
{
    switch (scope.kind)
    {
    case LookScope::Kind::Environment:
        return MapsUsingEnvironment(profile, scope.environment);
    case LookScope::Kind::Map:
        if (const MapLookOverrides* entry = profile.FindMap(scope.world))
        {
            return entry->maps;
        }
        break;
    case LookScope::Kind::Default:
        break;
    }
    return {};
}

std::string JoinMaps(const std::vector<int>& maps)
{
    std::string text;
    for (const int map : maps)
    {
        text += (text.empty() ? "" : ", ") + std::to_string(map);
    }
    return text;
}
} // namespace

LookEditOutcome ApplyLookEdits(LookProfileData& profile, std::span<const LookValueEdit> edits, const LookScope& scope,
                               std::string_view mapName)
{
    LookEditOutcome outcome;
    if (scope.kind == LookScope::Kind::Environment && scope.environment >= profile.environments.size())
    {
        outcome.error = NoEnvironment(scope.environment);
        return outcome;
    }
    LookProfileData work = profile;
    const LookValues before = ValuesOf(profile, scope);
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
        LookOverrides& target = scope.kind == LookScope::Kind::Map ? EntryOf(work, scope.world, mapName).overrides
                                : scope.kind == LookScope::Kind::Environment
                                    ? work.environments[scope.environment].overrides
                                    : work.defaultOverrides;
        target[edit.value] = edit.number;
    }
    outcome.entryMaps = MapsReached(work, scope);
    Commit(profile, std::move(work), outcome);
    return outcome;
}

LookEditOutcome ApplyLookEdits(LookProfileData& profile, std::span<const LookValueEdit> edits, std::optional<int> world,
                               std::string_view mapName)
{
    return ApplyLookEdits(profile, edits, world ? LookScope::Map(*world) : LookScope::Default(), mapName);
}

LookEditOutcome ClearLookValue(LookProfileData& profile, LookValue value, const LookScope& scope)
{
    LookEditOutcome outcome;
    LookProfileData work = profile;
    switch (scope.kind)
    {
    case LookScope::Kind::Default:
        work.defaultOverrides[value].reset();
        break;
    case LookScope::Kind::Environment:
        if (scope.environment >= work.environments.size())
        {
            outcome.error = NoEnvironment(scope.environment);
            return outcome;
        }
        work.environments[scope.environment].overrides[value].reset();
        break;
    case LookScope::Kind::Map:
        if (MapLookOverrides* entry = work.FindMap(scope.world))
        {
            entry->overrides[value].reset();
            DropIfEmpty(work, scope.world);
        }
        break;
    }
    outcome.oldValues.push_back(ValuesOf(profile, scope)[value]);
    outcome.entryMaps = MapsReached(work, scope);
    Commit(profile, std::move(work), outcome);
    return outcome;
}

std::vector<int> MapsUsingEnvironment(const LookProfileData& profile, std::size_t environment)
{
    std::vector<int> maps;
    for (const MapLookOverrides& entry : profile.maps)
    {
        if (entry.environment == environment)
        {
            maps.insert(maps.end(), entry.maps.begin(), entry.maps.end());
        }
    }
    std::sort(maps.begin(), maps.end());
    return maps;
}

LookEditOutcome SetMapEnvironment(LookProfileData& profile, int world, std::optional<std::size_t> environment,
                                  std::string_view mapName)
{
    LookEditOutcome outcome;
    if (environment && *environment >= profile.environments.size())
    {
        outcome.error = NoEnvironment(*environment);
        return outcome;
    }
    LookProfileData work = profile;
    MapLookOverrides& entry = EntryOf(work, world, mapName);
    entry.environment = environment;
    outcome.entryMaps = entry.maps;
    DropIfEmpty(work, world);
    Commit(profile, std::move(work), outcome);
    return outcome;
}

LookEditOutcome AddEnvironment(LookProfileData& profile, std::string_view name)
{
    LookEditOutcome outcome;
    LookProfileData work = profile;
    work.environments.push_back({std::string(name), {}});
    Commit(profile, std::move(work), outcome);
    return outcome;
}

LookEditOutcome RenameEnvironment(LookProfileData& profile, std::size_t environment, std::string_view name)
{
    LookEditOutcome outcome;
    if (environment >= profile.environments.size())
    {
        outcome.error = NoEnvironment(environment);
        return outcome;
    }
    LookProfileData work = profile;
    work.environments[environment].name = name;
    outcome.entryMaps = MapsUsingEnvironment(work, environment);
    Commit(profile, std::move(work), outcome);
    return outcome;
}

LookEditOutcome RemoveEnvironment(LookProfileData& profile, std::size_t environment)
{
    LookEditOutcome outcome;
    if (environment >= profile.environments.size())
    {
        outcome.error = NoEnvironment(environment);
        return outcome;
    }
    if (const std::vector<int> users = MapsUsingEnvironment(profile, environment); !users.empty())
    {
        outcome.error =
            "environment \"" + profile.environments[environment].name + "\" is used by maps " + JoinMaps(users);
        return outcome;
    }
    LookProfileData work = profile;
    work.environments.erase(work.environments.begin() + static_cast<std::ptrdiff_t>(environment));
    for (MapLookOverrides& entry : work.maps)
    {
        if (entry.environment && *entry.environment > environment)
        {
            --*entry.environment;
        }
    }
    Commit(profile, std::move(work), outcome);
    return outcome;
}

LookEditOutcome ReplaceLookProfile(LookProfileData& profile, const LookProfileData& replacement)
{
    LookEditOutcome outcome;
    Commit(profile, replacement, outcome);
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
