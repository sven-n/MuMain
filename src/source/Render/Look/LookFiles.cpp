#include "stdafx.h"

#include "Render/Look/LookFiles.h"

#include <algorithm>
#include <sstream>

namespace Render::Look
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Maps::LightMoodValueInfo;
using Data::Maps::LookValue;
using Data::Maps::LookValueInfo;

constexpr const char* DefaultKey = "default";
constexpr const char* MapsKey = "maps";
constexpr const char* WorldKey = "world";
constexpr const char* NameKey = "name";
constexpr int NoWorld = -1;

void AddWarning(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& message)
{
    issues.push_back(
        {ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, "", message});
}

void AddFatal(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& message)
{
    Data::Items::Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, "", message);
}

std::string FormatNumber(double value)
{
    std::ostringstream text;
    text << value;
    return text.str();
}

std::string FatalMessages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.severity == ItemDataIssueSeverity::Fatal)
        {
            text += (text.empty() ? "" : "; ") + issue.ToString();
        }
    }
    return text;
}

bool ReadLook(const std::string& text, const std::string& source, LookFiles& files, std::vector<ItemDataIssue>& issues)
{
    std::vector<ItemDataIssue> read;
    Data::Maps::ReadLookProfileJson(text, source, files.lookData, read);
    const bool valid = !Data::Items::HasFatal(read);
    issues.insert(issues.end(), read.begin(), read.end());
    files.lookUsable = valid;
    files.look = valid ? LookDocument::parse(text) : LookDocument{};
    return valid;
}

bool ReadMood(const std::string& text, const std::string& source, LookFiles& files, std::vector<ItemDataIssue>& issues)
{
    std::vector<ItemDataIssue> read;
    Data::Maps::ReadLightMoodJson(text, source, files.moodData, read);
    const bool valid = !Data::Items::HasFatal(read);
    issues.insert(issues.end(), read.begin(), read.end());
    files.moodUsable = valid;
    files.mood = valid ? LookDocument::parse(text) : LookDocument{};
    return valid;
}

LookDocument EmptyLookDocument()
{
    LookDocument document = LookDocument::object();
    document[Data::Items::Json::Keys::FormatVersion] = Data::Maps::LookProfileFormatVersion;
    return document;
}

void LoadShippedMood(LookFileSystem& fileSystem, const std::filesystem::path& moodFile, LookFiles& files,
                     std::vector<ItemDataIssue>& issues)
{
    const std::string source = moodFile.generic_string();
    const std::optional<std::string> text = fileSystem.Read(moodFile);
    if (!text)
    {
        AddFatal(issues, source, "could not be read");
        files.moodUsable = false;
        files.mood = LookDocument{};
        files.moodData = {};
        return;
    }
    (void)ReadMood(*text, source, files, issues);
}

struct ResolvedName
{
    std::optional<LookValue> look;
    const LightMoodValueInfo* mood = nullptr;
    // The key in its file.
    std::string key;
};

std::optional<ResolvedName> Resolve(std::string_view name)
{
    if (name.starts_with(MoodNamePrefix))
    {
        const std::string_view key = name.substr(MoodNamePrefix.size());
        const LightMoodValueInfo* info = Data::Maps::FindLightMoodValue(key);
        if (!info)
        {
            return std::nullopt;
        }
        return ResolvedName{std::nullopt, info, std::string(key)};
    }
    const std::optional<LookValue> value = Data::Maps::FindLookValue(name);
    if (!value)
    {
        return std::nullopt;
    }
    return ResolvedName{value, nullptr, std::string(name)};
}

LookDocument* FindMapEntry(LookDocument& document, int world)
{
    const auto maps = document.find(MapsKey);
    if (maps == document.end() || !maps->is_array())
    {
        return nullptr;
    }
    for (LookDocument& entry : *maps)
    {
        const auto entryWorld = entry.find(WorldKey);
        if (entryWorld != entry.end() && entryWorld->is_number_integer() && entryWorld->get<int>() == world)
        {
            return &entry;
        }
    }
    return nullptr;
}

const LookDocument* FindMapEntry(const LookDocument& document, int world)
{
    return FindMapEntry(const_cast<LookDocument&>(document), world);
}

LookDocument& MapEntry(LookDocument& document, int world, std::string_view mapName)
{
    if (LookDocument* entry = FindMapEntry(document, world))
    {
        return *entry;
    }
    if (!document.contains(MapsKey) || !document[MapsKey].is_array())
    {
        document[MapsKey] = LookDocument::array();
    }
    LookDocument entry = LookDocument::object();
    entry[WorldKey] = world;
    if (!mapName.empty())
    {
        entry[NameKey] = std::string(mapName);
    }
    document[MapsKey].push_back(std::move(entry));
    return document[MapsKey].back();
}

LookDocument& DefaultEntry(LookDocument& document)
{
    if (!document.contains(DefaultKey) || !document[DefaultKey].is_object())
    {
        document[DefaultKey] = LookDocument::object();
    }
    return document[DefaultKey];
}

bool HasKey(const LookDocument* entry, const std::string& key)
{
    return entry && entry->is_object() && entry->contains(key);
}

LookValueSource SourceOf(const LookDocument& document, bool usable, const std::string& key, int world)
{
    if (!usable)
    {
        return LookValueSource::BuiltIn;
    }
    if (HasKey(FindMapEntry(document, world), key))
    {
        return LookValueSource::Map;
    }
    const auto defaults = document.find(DefaultKey);
    if (defaults != document.end() && HasKey(&*defaults, key))
    {
        return LookValueSource::Default;
    }
    return LookValueSource::BuiltIn;
}

double EffectiveValue(const LookFiles& files, const ResolvedName& name, int world)
{
    if (name.look)
    {
        return files.lookData.ForWorld(world)[*name.look];
    }
    return files.moodData.ForWorld(world).*(name.mood->member);
}

// Re-reads the edited documents through the files' readers; the error text when one is invalid.
std::string Revalidate(LookFiles& files, bool lookEdited, bool moodEdited)
{
    std::vector<ItemDataIssue> issues;
    if (lookEdited)
    {
        std::vector<ItemDataIssue> read;
        Data::Maps::ReadLookProfileJson(files.look.dump(), "Look.json", files.lookData, read);
        issues.insert(issues.end(), read.begin(), read.end());
    }
    if (moodEdited)
    {
        std::vector<ItemDataIssue> read;
        Data::Maps::ReadLightMoodJson(files.mood.dump(), "LightMood.json", files.moodData, read);
        issues.insert(issues.end(), read.begin(), read.end());
    }
    return FatalMessages(issues);
}

std::string UnusableError(const ResolvedName& name)
{
    return name.look ? std::string("Look.json is unusable (see the log); fix it and $look load")
                     : std::string("LightMood.json is unusable (see the log); fix it and $look load");
}

// A map entry with nothing but its world and name, that the loaded file did not have.
void DropEmptyNewEntry(LookDocument& document, const LookDocument& loaded, int world)
{
    LookDocument* entry = FindMapEntry(document, world);
    if (!entry || FindMapEntry(loaded, world))
    {
        return;
    }
    for (const auto& [key, value] : entry->items())
    {
        if (key != WorldKey && key != NameKey)
        {
            return;
        }
    }
    LookDocument& maps = document[MapsKey];
    for (std::size_t index = 0; index < maps.size(); ++index)
    {
        if (&maps[index] == entry)
        {
            maps.erase(index);
            return;
        }
    }
}

// Sets `key` of `target` to its value in `source`, or removes it when `source` has none.
void RestoreKey(LookDocument& target, const LookDocument* source, const std::string& key)
{
    if (HasKey(source, key))
    {
        target[key] = (*source)[key];
    }
    else if (target.is_object())
    {
        target.erase(key);
    }
}

std::string Inline(const LookDocument& value)
{
    if (!value.is_object() || value.empty())
    {
        return value.dump();
    }
    std::string text = "{ ";
    bool first = true;
    for (const auto& [key, member] : value.items())
    {
        text += (first ? "" : ", ") + LookDocument(key).dump() + ": " + member.dump();
        first = false;
    }
    return text + " }";
}

struct SaveTarget
{
    std::filesystem::path file;
    std::string text;
    std::filesystem::path temporary;
    std::filesystem::path backup;
    bool hadOriginal = false;
};

std::filesystem::path WithSuffix(const std::filesystem::path& file, const char* suffix)
{
    std::filesystem::path path = file;
    path += suffix;
    return path;
}

void RemoveAll(LookFileSystem& fileSystem, const std::vector<SaveTarget>& targets, bool temporaries, bool backups)
{
    for (const SaveTarget& target : targets)
    {
        if (temporaries)
        {
            (void)fileSystem.Remove(target.temporary);
        }
        if (backups && target.hadOriginal)
        {
            (void)fileSystem.Remove(target.backup);
        }
    }
}
} // namespace

LookFilesLoad LoadLookFiles(LookFileSystem& fileSystem, const std::filesystem::path& lookFile,
                            const std::filesystem::path& moodFile)
{
    LookFilesLoad result;
    const std::string lookSource = lookFile.generic_string();
    if (!fileSystem.Exists(lookFile))
    {
        AddWarning(result.issues, lookSource, "not found, the built-in values apply");
        result.files.look = EmptyLookDocument();
        result.files.lookUsable = true;
    }
    else if (const std::optional<std::string> text = fileSystem.Read(lookFile))
    {
        (void)ReadLook(*text, lookSource, result.files, result.issues);
    }
    else
    {
        AddFatal(result.issues, lookSource, "could not be read");
        result.files.lookUsable = false;
    }
    LoadShippedMood(fileSystem, moodFile, result.files, result.issues);
    return result;
}

std::optional<LookFiles> LoadCandidateFiles(LookFileSystem& fileSystem, const std::filesystem::path& lookFile,
                                            const std::filesystem::path& candidateMoodFile,
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
    bool valid = ReadLook(*lookText, lookSource, files, issues);
    if (fileSystem.Exists(candidateMoodFile))
    {
        const std::string moodSource = candidateMoodFile.generic_string();
        const std::optional<std::string> moodText = fileSystem.Read(candidateMoodFile);
        if (!moodText)
        {
            AddFatal(issues, moodSource, "could not be read");
            valid = false;
        }
        else
        {
            valid = ReadMood(*moodText, moodSource, files, issues) && valid;
        }
    }
    else
    {
        // Never a previously loaded candidate's mood: the shipped file, read again.
        LoadShippedMood(fileSystem, shippedMoodFile, files, issues);
    }
    if (!valid)
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
    for (const LightMoodValueInfo& info : Data::Maps::LightMoodValueInfos())
    {
        names.push_back(std::string(MoodNamePrefix) + info.key);
    }
    return names;
}

std::vector<LookValueState> DescribeLookValues(const LookFiles& files, int world, std::optional<std::string_view> name)
{
    std::vector<LookValueState> states;
    for (const std::string& each : LookValueNames())
    {
        if (name && *name != each)
        {
            continue;
        }
        const ResolvedName resolved = *Resolve(each);
        LookValueState state{each, EffectiveValue(files, resolved, world)};
        if (resolved.look)
        {
            const LookValueInfo& info = Data::Maps::Info(*resolved.look);
            state.source = SourceOf(files.look, files.lookUsable, resolved.key, world);
            state.min = info.min;
            state.max = info.max;
        }
        else
        {
            state.source = SourceOf(files.mood, files.moodUsable, resolved.key, world);
            state.min = resolved.mood->min;
            state.max = resolved.mood->max;
        }
        states.push_back(std::move(state));
    }
    return states;
}

LookEditResult SetLookValues(LookFiles& files, std::span<const LookEdit> edits, std::optional<int> world,
                             std::string_view mapName)
{
    LookEditResult result;
    LookFiles work = files;
    const int valueWorld = world.value_or(NoWorld);
    bool lookEdited = false;
    bool moodEdited = false;
    for (const LookEdit& edit : edits)
    {
        const std::optional<ResolvedName> resolved = Resolve(edit.name);
        if (!resolved)
        {
            result.error = edit.name + ": is not a look value";
            return result;
        }
        const bool usable = resolved->look ? work.lookUsable : work.moodUsable;
        if (!usable)
        {
            result.error = edit.name + ": " + UnusableError(*resolved);
            return result;
        }
        const double min = resolved->look ? Data::Maps::Info(*resolved->look).min : resolved->mood->min;
        const double max = resolved->look ? Data::Maps::Info(*resolved->look).max : resolved->mood->max;
        if (!(edit.value >= min && edit.value <= max))
        {
            result.error = edit.name + ": " + FormatNumber(edit.value) + " is outside " + FormatNumber(min) + " to " +
                           FormatNumber(max);
            return result;
        }
        result.oldValues.push_back(EffectiveValue(files, *resolved, valueWorld));
        LookDocument& document = resolved->look ? work.look : work.mood;
        LookDocument& entry = world ? MapEntry(document, *world, mapName) : DefaultEntry(document);
        entry[resolved->key] = edit.value;
        lookEdited = lookEdited || resolved->look.has_value();
        moodEdited = moodEdited || !resolved->look.has_value();
    }
    const std::string errors = Revalidate(work, lookEdited, moodEdited);
    if (!errors.empty())
    {
        result.error = errors;
        result.oldValues.clear();
        return result;
    }
    files = std::move(work);
    result.applied = true;
    result.moodChanged = moodEdited;
    return result;
}

LookEditResult ResetLookValues(LookFiles& files, const LookFiles& loaded, std::optional<std::string_view> name,
                               int world)
{
    LookEditResult result;
    if (!name)
    {
        files = loaded;
        result.applied = true;
        result.moodChanged = true;
        return result;
    }
    const std::optional<ResolvedName> resolved = Resolve(*name);
    if (!resolved)
    {
        result.error = std::string(*name) + ": is not a look value";
        return result;
    }
    if (!(resolved->look ? files.lookUsable && loaded.lookUsable : files.moodUsable && loaded.moodUsable))
    {
        result.error = std::string(*name) + ": " + UnusableError(*resolved);
        return result;
    }
    LookFiles work = files;
    LookDocument& document = resolved->look ? work.look : work.mood;
    const LookDocument& loadedDocument = resolved->look ? loaded.look : loaded.mood;
    result.oldValues.push_back(EffectiveValue(files, *resolved, world));

    const auto loadedDefault = loadedDocument.find(DefaultKey);
    RestoreKey(DefaultEntry(document), loadedDefault != loadedDocument.end() ? &*loadedDefault : nullptr,
               resolved->key);
    const LookDocument* loadedEntry = FindMapEntry(loadedDocument, world);
    if (HasKey(loadedEntry, resolved->key))
    {
        LookDocument& entry = MapEntry(document, world, {});
        RestoreKey(entry, loadedEntry, resolved->key);
    }
    else if (LookDocument* entry = FindMapEntry(document, world))
    {
        entry->erase(resolved->key);
        DropEmptyNewEntry(document, loadedDocument, world);
    }

    const std::string errors = Revalidate(work, resolved->look.has_value(), !resolved->look.has_value());
    if (!errors.empty())
    {
        result.error = errors;
        result.oldValues.clear();
        return result;
    }
    files = std::move(work);
    result.applied = true;
    result.moodChanged = !resolved->look.has_value();
    return result;
}

std::string FormatLookDocument(const LookDocument& document)
{
    std::string text = "{\n";
    std::size_t index = 0;
    for (const auto& [key, value] : document.items())
    {
        text += "  " + LookDocument(key).dump() + ": ";
        if (value.is_object() && !value.empty())
        {
            text += "{\n";
            std::size_t member = 0;
            for (const auto& [memberKey, memberValue] : value.items())
            {
                text += "    " + LookDocument(memberKey).dump() + ": " + memberValue.dump();
                text += ++member < value.size() ? ",\n" : "\n";
            }
            text += "  }";
        }
        else if (value.is_array() && !value.empty())
        {
            text += "[\n";
            for (std::size_t element = 0; element < value.size(); ++element)
            {
                text += "    " + Inline(value[element]);
                text += element + 1 < value.size() ? ",\n" : "\n";
            }
            text += "  ]";
        }
        else
        {
            text += value.dump();
        }
        text += ++index < document.size() ? ",\n" : "\n";
    }
    return text + "}\n";
}

LookSaveResult SaveLookFiles(LookFileSystem& fileSystem, const LookFiles& state, const std::filesystem::path& lookFile,
                             const std::filesystem::path& moodFile)
{
    LookSaveResult result;
    std::vector<SaveTarget> targets;
    const auto addTarget = [&targets](const std::filesystem::path& file, const LookDocument& document)
    {
        SaveTarget target;
        target.file = file;
        target.text = FormatLookDocument(document);
        target.temporary = WithSuffix(file, ".tmp");
        target.backup = WithSuffix(file, ".bak");
        targets.push_back(std::move(target));
    };
    if (state.lookUsable)
    {
        addTarget(lookFile, state.look);
    }
    if (state.moodUsable)
    {
        addTarget(moodFile, state.mood);
    }

    // Temporaries, read back.
    for (const SaveTarget& target : targets)
    {
        if (!fileSystem.Write(target.temporary, target.text) || fileSystem.Read(target.temporary) != target.text)
        {
            RemoveAll(fileSystem, targets, true, false);
            result.message = "could not write " + target.temporary.generic_string() + "; nothing was saved";
            return result;
        }
    }
    // Backups of the originals.
    for (SaveTarget& target : targets)
    {
        target.hadOriginal = fileSystem.Exists(target.file);
        if (target.hadOriginal && !fileSystem.Copy(target.file, target.backup))
        {
            RemoveAll(fileSystem, targets, true, true);
            result.message = "could not back up " + target.file.generic_string() + "; nothing was saved";
            return result;
        }
    }
    // Into place, one after the other.
    for (std::size_t index = 0; index < targets.size(); ++index)
    {
        const SaveTarget& target = targets[index];
        if (fileSystem.Rename(target.temporary, target.file))
        {
            continue;
        }
        if (index == 0)
        {
            RemoveAll(fileSystem, targets, true, true);
            result.message = "could not replace " + target.file.generic_string() + "; nothing was saved";
            return result;
        }
        const SaveTarget& first = targets[0];
        const bool restored =
            first.hadOriginal ? fileSystem.Copy(first.backup, first.file) : fileSystem.Remove(first.file);
        if (!restored)
        {
            RemoveAll(fileSystem, targets, true, false);
            result.backupsKept = true;
            result.message = "could not replace " + target.file.generic_string() + " and could not restore " +
                             first.file.generic_string() +
                             ": the files are mixed; backups kept: " + first.backup.generic_string() + " (" +
                             first.file.generic_string() + "), " + target.backup.generic_string() + " (" +
                             target.file.generic_string() + ")";
            return result;
        }
        RemoveAll(fileSystem, targets, true, true);
        result.message = "could not replace " + target.file.generic_string() + "; " + first.file.generic_string() +
                         " restored, nothing was saved";
        return result;
    }
    RemoveAll(fileSystem, targets, false, true);
    result.saved = true;
    std::string names;
    for (const SaveTarget& target : targets)
    {
        names += (names.empty() ? "" : " and ") + target.file.generic_string();
    }
    result.message = "saved " + names;
    return result;
}

} // namespace Render::Look
