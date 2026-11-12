#include "stdafx.h"

#include "Render/Look/LookFiles.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MapData/LookProfileEdits.h"

#include <algorithm>

namespace Render::Look
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Maps::LookValue;
using Data::Maps::LookValueInfo;

constexpr const char* UnusableError = "Look.json is unusable (see the log); fix it and $look load";

void AddWarning(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& message)
{
    issues.push_back(
        {ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, "", message});
}

void AddFatal(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& message)
{
    Data::Items::Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, "", message);
}

// The profile of `text`; `formatVersion` gets the file's version. False when it is invalid.
bool ReadLook(const std::string& text, const std::string& source, LookFiles& files, int& formatVersion,
              std::vector<ItemDataIssue>& issues)
{
    std::vector<ItemDataIssue> read;
    Data::Maps::ReadLookProfileJson(text, source, files.lookData, read, &formatVersion);
    const bool valid = !Data::Items::HasFatal(read);
    issues.insert(issues.end(), read.begin(), read.end());
    files.lookUsable = valid;
    return valid;
}

// The moods of an old light mood file, if it is there and valid; otherwise why not, as an error.
std::optional<Data::Maps::LightMoodData> ReadLegacyMood(LookFileSystem& fileSystem, const std::filesystem::path& file,
                                                        std::vector<ItemDataIssue>& issues)
{
    const std::string source = file.generic_string();
    const std::optional<std::string> text = fileSystem.Exists(file) ? fileSystem.Read(file) : std::nullopt;
    if (!text)
    {
        AddFatal(issues, source, "not found or could not be read: Look.json is format 1 and needs it");
        return std::nullopt;
    }
    std::vector<ItemDataIssue> read;
    Data::Maps::LightMoodData moods;
    Data::Maps::ReadLightMoodJson(*text, source, moods, read);
    issues.insert(issues.end(), read.begin(), read.end());
    if (Data::Items::HasFatal(read))
    {
        return std::nullopt;
    }
    return moods;
}

// Gives a format 1 profile its moods; false (the profile unusable) when they break its rules.
bool Upgrade(LookFiles& files, const Data::Maps::LightMoodData& moods, const std::string& source,
             std::vector<ItemDataIssue>& issues)
{
    Data::Maps::UpgradeLightMood(files.lookData, moods);
    if (Data::Maps::ValidateLookProfile(files.lookData, source, issues))
    {
        return true;
    }
    files.lookData = {};
    files.lookUsable = false;
    return false;
}

void WarnStaleMood(LookFileSystem& fileSystem, const std::filesystem::path& moodFile,
                   std::vector<ItemDataIssue>& issues)
{
    if (fileSystem.Exists(moodFile))
    {
        AddWarning(issues, moodFile.generic_string(),
                   "is no longer read: the light mood is in Look.json (mood.*); it can be deleted");
    }
}

std::optional<LookValue> Resolve(std::string_view name)
{
    return Data::Maps::FindLookValue(name);
}

LookValueSource SourceOf(const LookFiles& files, LookValue value, int world)
{
    if (!files.lookUsable)
    {
        return LookValueSource::BuiltIn;
    }
    const Data::Maps::LookProfileData& profile = files.lookData;
    if (const Data::Maps::MapLookOverrides* entry = profile.FindMap(world))
    {
        if (entry->overrides[value])
        {
            return LookValueSource::Map;
        }
        if (entry->environment && *entry->environment < profile.environments.size() &&
            profile.environments[*entry->environment].overrides[value])
        {
            return LookValueSource::Environment;
        }
    }
    return profile.defaultOverrides[value] ? LookValueSource::Default : LookValueSource::BuiltIn;
}

LookEditResult ResultOf(Data::Maps::LookEditOutcome outcome)
{
    return {outcome.applied, std::move(outcome.error), std::move(outcome.oldValues), outcome.moodChanged,
            std::move(outcome.entryMaps)};
}

std::filesystem::path WithSuffix(const std::filesystem::path& file, const char* suffix)
{
    std::filesystem::path path = file;
    path += suffix;
    return path;
}
} // namespace

LookFilesLoad LoadLookFiles(LookFileSystem& fileSystem, const std::filesystem::path& lookFile,
                            const std::filesystem::path& legacyMoodFile)
{
    LookFilesLoad result;
    const std::string lookSource = lookFile.generic_string();
    if (!fileSystem.Exists(lookFile))
    {
        AddWarning(result.issues, lookSource, "not found, the built-in values apply");
        WarnStaleMood(fileSystem, legacyMoodFile, result.issues);
        return result;
    }
    const std::optional<std::string> text = fileSystem.Read(lookFile);
    if (!text)
    {
        AddFatal(result.issues, lookSource, "could not be read");
        result.files.lookUsable = false;
        return result;
    }
    int formatVersion = 0;
    if (!ReadLook(*text, lookSource, result.files, formatVersion, result.issues))
    {
        return result;
    }
    if (formatVersion != 1)
    {
        WarnStaleMood(fileSystem, legacyMoodFile, result.issues);
        return result;
    }
    if (const std::optional<Data::Maps::LightMoodData> moods =
            ReadLegacyMood(fileSystem, legacyMoodFile, result.issues))
    {
        (void)Upgrade(result.files, *moods, lookSource, result.issues);
        return result;
    }
    // The look values still apply, with the Classic mood; saving would make that mood the file's.
    result.files.saveBlocked = "Look.json is format 1 and its light mood (" + legacyMoodFile.generic_string() +
                               ") is missing or invalid; restore it, or write Look.json as format 2 with its mood.* "
                               "values (docs/enhanced-look.md)";
    return result;
}

std::optional<LookFiles> LoadCandidateFiles(LookFileSystem& fileSystem, const std::filesystem::path& lookFile,
                                            const std::filesystem::path& candidateMoodFile,
                                            const std::filesystem::path& shippedLookFile,
                                            const std::filesystem::path& shippedMoodFile,
                                            std::vector<ItemDataIssue>& issues)
{
    LookFiles files;
    const std::string lookSource = lookFile.generic_string();
    const std::optional<std::string> lookText = fileSystem.Exists(lookFile) ? fileSystem.Read(lookFile) : std::nullopt;
    if (!lookText)
    {
        AddFatal(issues, lookSource, "not found or could not be read");
        return std::nullopt;
    }
    int formatVersion = 0;
    if (!ReadLook(*lookText, lookSource, files, formatVersion, issues))
    {
        return std::nullopt;
    }
    if (formatVersion != 1)
    {
        WarnStaleMood(fileSystem, candidateMoodFile, issues);
        return files;
    }
    std::optional<Data::Maps::LightMoodData> moods;
    if (fileSystem.Exists(candidateMoodFile))
    {
        moods = ReadLegacyMood(fileSystem, candidateMoodFile, issues);
    }
    else
    {
        // Never a previously loaded candidate's mood: the shipped profile's, read again, to look at. Its
        // environments are not the candidate's, so each map's mood is copied: saving would keep that copy.
        const LookFilesLoad shipped = LoadLookFiles(fileSystem, shippedLookFile, shippedMoodFile);
        if (shipped.files.lookUsable && !shipped.files.saveBlocked)
        {
            moods = Data::Maps::MoodsOf(shipped.files.lookData);
            files.saveBlocked = "candidate " + lookSource +
                                " is format 1 without its LightMood.json: it shows the shipped moods and cannot be "
                                "saved; add its LightMood.json, or write it as format 2 with its mood.* values";
        }
        else
        {
            AddFatal(issues, lookSource,
                     "is format 1 without a LightMood.json, and the shipped light mood is unusable");
        }
    }
    if (!moods || !Upgrade(files, *moods, lookSource, issues))
    {
        return std::nullopt;
    }
    return files;
}

const char* ToString(LookValueSource source)
{
    switch (source)
    {
    case LookValueSource::Default:
        return "default";
    case LookValueSource::Environment:
        return "environment";
    case LookValueSource::Map:
        return "map";
    case LookValueSource::Personal:
        return "player";
    case LookValueSource::BuiltIn:
        break;
    }
    return "built-in";
}

std::vector<std::string> LookValueNames()
{
    std::vector<std::string> names;
    for (const LookValueInfo& info : Data::Maps::LookValueInfos())
    {
        names.emplace_back(info.name);
    }
    return names;
}

std::vector<LookValueState> DescribeLookValues(const LookFiles& files, int world, std::optional<std::string_view> name)
{
    const Data::Maps::LookValues values = files.lookData.ForWorld(world);
    std::vector<LookValueState> states;
    for (const LookValueInfo& info : Data::Maps::LookValueInfos())
    {
        if (name && *name != info.name)
        {
            continue;
        }
        states.push_back({info.name, values[info.value], SourceOf(files, info.value, world), info.min, info.max});
    }
    return states;
}

LookEditResult SetLookValues(LookFiles& files, std::span<const LookEdit> edits, std::optional<int> world,
                             std::string_view mapName)
{
    std::vector<Data::Maps::LookValueEdit> changes;
    for (const LookEdit& edit : edits)
    {
        const std::optional<LookValue> value = Resolve(edit.name);
        if (!value)
        {
            return {false, edit.name + ": is not a look value"};
        }
        if (!files.lookUsable)
        {
            return {false, edit.name + ": " + UnusableError};
        }
        changes.push_back({*value, edit.value});
    }
    return ResultOf(Data::Maps::ApplyLookEdits(files.lookData, changes, world, mapName));
}

LookEditResult ResetLookValues(LookFiles& files, const LookFiles& loaded, std::optional<std::string_view> name,
                               int world)
{
    if (!name)
    {
        // The mood rebuild also reads look values (the highlight knee): a full reset always runs it.
        files = loaded;
        return {true, {}, {}, true};
    }
    const std::optional<LookValue> value = Resolve(*name);
    if (!value)
    {
        return {false, std::string(*name) + ": is not a look value"};
    }
    if (!files.lookUsable || !loaded.lookUsable)
    {
        return {false, std::string(*name) + ": " + UnusableError};
    }
    return ResultOf(Data::Maps::ResetLookValue(files.lookData, loaded.lookData, *value, world));
}

LookSaveResult SaveLookFiles(LookFileSystem& fileSystem, const LookFiles& state, const std::filesystem::path& lookFile)
{
    LookSaveResult result;
    if (!state.lookUsable)
    {
        result.message = UnusableError;
        return result;
    }
    if (state.saveBlocked)
    {
        result.message = *state.saveBlocked;
        return result;
    }
    const std::string text = Data::Maps::WriteLookProfileJson(state.lookData);
    const std::filesystem::path temporary = WithSuffix(lookFile, ".tmp");
    const std::filesystem::path backup = WithSuffix(lookFile, ".bak");
    if (!fileSystem.Write(temporary, text) || fileSystem.Read(temporary) != text)
    {
        (void)fileSystem.Remove(temporary);
        result.message = "could not write " + temporary.generic_string() + "; nothing was saved";
        return result;
    }
    const bool hadOriginal = fileSystem.Exists(lookFile);
    if (hadOriginal && !fileSystem.Copy(lookFile, backup))
    {
        (void)fileSystem.Remove(temporary);
        (void)fileSystem.Remove(backup);
        result.message = "could not back up " + lookFile.generic_string() + "; nothing was saved";
        return result;
    }
    if (!fileSystem.Rename(temporary, lookFile))
    {
        (void)fileSystem.Remove(temporary);
        if (hadOriginal)
        {
            (void)fileSystem.Remove(backup);
        }
        result.message = "could not replace " + lookFile.generic_string() + "; nothing was saved";
        return result;
    }
    if (hadOriginal)
    {
        (void)fileSystem.Remove(backup);
    }
    result.saved = true;
    result.message = "saved " + lookFile.generic_string();
    return result;
}

} // namespace Render::Look
