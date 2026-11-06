#include "stdafx.h"

#include "MapObjectSoundsJson.h"

#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterModelValidation.h"

#include <algorithm>
#include <optional>

namespace Data::Maps
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* ObjectSounds = "objectSounds";
constexpr const char* Maps = "maps";
constexpr const char* Objects = "objects";
constexpr const char* Sound = "sound";
constexpr const char* File = "file";
constexpr const char* Name = "name";
constexpr const char* Channels = "channels";
constexpr const char* Plays = "plays";
} // namespace Keys

constexpr const char* WhileInView = "whileInView";

const Items::Json::KeySet EntryKeys{Keys::Maps, Keys::Objects, Keys::Sound, Keys::Plays};
const Items::Json::KeySet SoundKeys{Keys::File, Keys::Channels, Keys::Name};

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& source,
              const std::string& field, const std::string& message)
{
    issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
}

std::optional<int> ReadWholeIn(const OrderedJson& json, int low, int high)
{
    long long number = 0;
    if (!Items::Json::ReadWholeNumber(json, number) || number < low || number > high)
    {
        return std::nullopt;
    }
    return static_cast<int>(number);
}

std::string RangeText(int low, int high)
{
    return "must be a whole number from " + std::to_string(low) + " to " + std::to_string(high);
}

// The fields of one entry; false, with a fatal problem added, at the first
// value of the wrong form.
class EntryReader
{
public:
    EntryReader(size_t index, const std::string& source, std::vector<ItemDataIssue>& issues)
        : m_index(index), m_source(source), m_issues(issues)
    {
    }

    bool Read(const OrderedJson& json, MapObjectSound& sound)
    {
        if (!json.is_object() || !json.contains(Keys::Maps) || !json.contains(Keys::Sound))
        {
            return Fail("", "must be an object with maps and sound");
        }
        // No objects: any object of the maps (Q-MP8).
        sound.anyObject = !json.contains(Keys::Objects);
        return ReadMaps(json[Keys::Maps], sound.maps) &&
               (sound.anyObject || ReadObjects(json[Keys::Objects], sound.objects)) &&
               ReadSound(json[Keys::Sound], sound) && ReadPlays(json, sound.plays) && WarnUnknown(json);
    }

private:
    bool Fail(const std::string& key, const std::string& message)
    {
        AddIssue(m_issues, ItemDataIssueSeverity::Fatal, m_source, MapObjectSoundField(m_index, key), message);
        return false;
    }

    bool ReadMaps(const OrderedJson& json, std::vector<MapRef>& maps)
    {
        if (!ReadMapRefs(json, MapObjectSoundField(m_index, Keys::Maps), maps,
                         [&](const std::string& field, const std::string& message)
                         { AddIssue(m_issues, ItemDataIssueSeverity::Fatal, m_source, field, message); }))
        {
            return false;
        }
        return !maps.empty() || Fail(Keys::Maps, "must name one map or more");
    }

    bool ReadObjects(const OrderedJson& json, std::vector<int>& objects)
    {
        if (!json.is_array() || json.empty())
        {
            return Fail(Keys::Objects, "must be a list of one object type or more (left out: any object)");
        }
        for (size_t i = 0; i < json.size(); ++i)
        {
            const std::optional<int> type = ReadWholeIn(json[i], 0, MaxMapObjectType);
            if (!type)
            {
                return Fail(std::string(Keys::Objects) + "[" + std::to_string(i) + "]", RangeText(0, MaxMapObjectType));
            }
            if (std::find(objects.begin(), objects.end(), *type) != objects.end())
            {
                return Fail(Keys::Objects, "has " + std::to_string(*type) + " more than once");
            }
            objects.push_back(*type);
        }
        std::sort(objects.begin(), objects.end());
        return true;
    }

    bool ReadSound(const OrderedJson& json, MapObjectSound& sound)
    {
        const std::string key = Keys::Sound;
        if (!json.is_object() || !json.contains(Keys::File))
        {
            return Fail(key, "must be an object with file");
        }
        const OrderedJson& file = json[Keys::File];
        if (!file.is_string() || file.get_ref<const std::string&>().empty())
        {
            return Fail(key + "." + Keys::File, "must be a file below the client folder");
        }
        sound.file = file.get<std::string>();
        if (const std::string problem = Paths::FindPathProblem(sound.file); !problem.empty())
        {
            return Fail(key + "." + Keys::File, problem);
        }
        if (const auto channels = json.find(Keys::Channels); channels != json.end())
        {
            const std::optional<int> count = ReadWholeIn(*channels, 1, MaxMapSoundChannels);
            if (!count)
            {
                return Fail(key + "." + Keys::Channels, RangeText(1, MaxMapSoundChannels));
            }
            sound.channels = *count;
        }
        if (const auto name = json.find(Keys::Name); name != json.end())
        {
            if (!name->is_string())
            {
                return Fail(key + "." + Keys::Name, "must be a text");
            }
            sound.name = name->get<std::string>();
        }
        Items::Json::WarnAboutUnknownKeys(
            json, SoundKeys, MapObjectSoundField(m_index, key) + ".",
            [&](const std::string& field, const std::string& message)
            { AddIssue(m_issues, ItemDataIssueSeverity::Warning, m_source, field, message); });
        return true;
    }

    bool ReadPlays(const OrderedJson& json, MapObjectSoundPlay& plays)
    {
        const auto value = json.find(Keys::Plays);
        if (value == json.end())
        {
            return true;
        }
        if (!value->is_string() || value->get_ref<const std::string&>() != WhileInView)
        {
            return Fail(Keys::Plays, std::string("must be ") + WhileInView);
        }
        plays = MapObjectSoundPlay::WhileInView;
        return true;
    }

    bool WarnUnknown(const OrderedJson& json)
    {
        Items::Json::WarnAboutUnknownKeys(
            json, EntryKeys, MapObjectSoundField(m_index, "") + ".",
            [&](const std::string& field, const std::string& message)
            { AddIssue(m_issues, ItemDataIssueSeverity::Warning, m_source, field, message); });
        return true;
    }

    size_t m_index;
    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

OrderedJson WriteEntry(const MapObjectSound& sound)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Maps] = WriteMapRefs(sound.maps);
    if (!sound.anyObject)
    {
        json[Keys::Objects] = sound.objects;
    }
    OrderedJson file = OrderedJson::object();
    file[Keys::File] = sound.file;
    file[Keys::Channels] = sound.channels;
    if (!sound.name.empty())
    {
        file[Keys::Name] = sound.name;
    }
    json[Keys::Sound] = std::move(file);
    json[Keys::Plays] = WhileInView;
    return json;
}
} // namespace

std::string MapObjectSoundField(size_t entry, std::string_view key)
{
    std::string field = std::string(Keys::ObjectSounds) + "[" + std::to_string(entry) + "]";
    return key.empty() ? field : field + "." + std::string(key);
}

void ReadMapObjectSoundsJson(std::string_view text, const std::string& source, std::vector<MapObjectSound>& sounds,
                             std::vector<Items::ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, MapObjectSoundsFormatVersion, root, issues))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, Keys::ObjectSounds}, "",
                                      [&](const std::string& field, const std::string& message)
                                      { AddIssue(issues, ItemDataIssueSeverity::Warning, source, field, message); });
    const auto entries = root.find(Keys::ObjectSounds);
    if (entries == root.end() || !entries->is_array())
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, source, Keys::ObjectSounds,
                 "missing or not a list of object sounds");
        return;
    }
    for (size_t index = 0; index < entries->size(); ++index)
    {
        MapObjectSound sound;
        if (EntryReader(index, source, issues).Read((*entries)[index], sound))
        {
            sounds.push_back(std::move(sound));
        }
    }
}

std::string WriteMapObjectSoundsJson(std::span<const MapObjectSound> sounds)
{
    OrderedJson entries = OrderedJson::array();
    for (const MapObjectSound& sound : sounds)
    {
        entries.push_back(WriteEntry(sound));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = MapObjectSoundsFormatVersion;
    root[Keys::ObjectSounds] = std::move(entries);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    return Items::Json::PutListEntriesOnOneLine(text, Keys::ObjectSounds) + "\n";
}

void ValidateMapObjectSounds(std::span<const MapObjectSound> sounds, const std::string& source,
                             std::vector<Items::ItemDataIssue>& issues)
{
    for (size_t entry = 0; entry < sounds.size(); ++entry)
    {
        const MapObjectSound& sound = sounds[entry];
        for (size_t other = 0; other < entry; ++other)
        {
            const MapObjectSound& earlier = sounds[other];
            const auto sharedObject = std::find_first_of(sound.objects.begin(), sound.objects.end(),
                                                         earlier.objects.begin(), earlier.objects.end());
            if (sharedObject == sound.objects.end())
            {
                continue;
            }
            for (const MapRef& map : sound.maps)
            {
                const auto sharedMap =
                    std::find_if(earlier.maps.begin(), earlier.maps.end(),
                                 [&](const MapRef& earlierMap) { return Overlaps(map, earlierMap); });
                if (sharedMap != earlier.maps.end())
                {
                    AddIssue(issues, ItemDataIssueSeverity::Fatal, source, MapObjectSoundField(entry, Keys::Objects),
                             "object " + std::to_string(*sharedObject) + " on map " + std::to_string(map.map) +
                                 " is in objectSounds[" + std::to_string(other) +
                                 "] too: one object type plays one sound on a map");
                    break;
                }
            }
        }
    }
}

void ValidateMapObjectSoundFiles(std::span<const MapObjectSound> sounds, const std::string& source,
                                 std::vector<Items::ItemDataIssue>& issues)
{
    for (size_t entry = 0; entry < sounds.size(); ++entry)
    {
        if (!Monsters::IsInClientFolder(sounds[entry].file))
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, source,
                     MapObjectSoundField(entry, std::string(Keys::Sound) + "." + Keys::File),
                     sounds[entry].file + " is not in the client folder: the sound is silent");
        }
    }
}
} // namespace Data::Maps
