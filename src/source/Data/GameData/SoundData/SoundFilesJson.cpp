#include "stdafx.h"

#include "SoundFilesJson.h"

#include "Audio/SoundIds.h"
#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterModelValidation.h"

#include <array>
#include <utility>

namespace Data::Sounds
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* Sounds = "sounds";
constexpr const char* Id = "id";
constexpr const char* Name = "name";
constexpr const char* File = "file";
constexpr const char* Channels = "channels";
constexpr const char* Spatial = "spatial";
constexpr const char* Type = "type";
} // namespace Keys

const Items::Json::KeySet EntryKeys{Keys::Id, Keys::Name, Keys::File, Keys::Channels, Keys::Spatial, Keys::Type};

constexpr std::array<std::pair<::Audio::SoundKind, std::string_view>, ::Audio::SoundKindCount> TypeWords{{
    {::Audio::SoundKind::Music, "music"},
    {::Audio::SoundKind::MapAmbience, "mapAmbience"},
    {::Audio::SoundKind::MapObject, "mapObject"},
    {::Audio::SoundKind::Monster, "monster"},
    {::Audio::SoundKind::Npc, "npc"},
    {::Audio::SoundKind::Hero, "hero"},
    {::Audio::SoundKind::Player, "player"},
    {::Audio::SoundKind::Effect, "effect"},
    {::Audio::SoundKind::Item, "item"},
    {::Audio::SoundKind::Creature, "creature"},
    {::Audio::SoundKind::Interface, "interface"},
    {::Audio::SoundKind::Other, "other"},
}};

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& source,
              const std::string& field, const std::string& message)
{
    issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
}

std::string TypeList()
{
    std::string list;
    for (const auto& [type, word] : TypeWords)
    {
        list += (list.empty() ? "" : ", ") + std::string(word);
    }
    return list;
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

    bool Read(const OrderedJson& json, SoundFile& sound)
    {
        if (!json.is_object() || !json.contains(Keys::Id) || !json.contains(Keys::File))
        {
            return Fail("", "must be an object with id and file");
        }
        return ReadId(json[Keys::Id], sound) && ReadName(json, sound) && ReadFile(json[Keys::File], sound) &&
               ReadChannels(json, sound) && ReadSpatial(json, sound) && ReadType(json, sound) && WarnUnknown(json);
    }

private:
    bool Fail(const std::string& key, const std::string& message)
    {
        AddIssue(m_issues, ItemDataIssueSeverity::Fatal, m_source, SoundFileField(m_index, key), message);
        return false;
    }

    bool ReadId(const OrderedJson& json, SoundFile& sound)
    {
        if (!json.is_string())
        {
            return Fail(Keys::Id, "must be a SOUND_ id");
        }
        sound.id = json.get<std::string>();
        const std::optional<ESound> number = ::Audio::SoundIds::Find(sound.id);
        if (!number)
        {
            return Fail(Keys::Id, sound.id + " is no SOUND_ id of the game");
        }
        sound.number = static_cast<int>(*number);
        return true;
    }

    bool ReadName(const OrderedJson& json, SoundFile& sound)
    {
        const auto name = json.find(Keys::Name);
        if (name == json.end())
        {
            return true;
        }
        if (!name->is_string())
        {
            return Fail(Keys::Name, "must be a text");
        }
        sound.name = name->get<std::string>();
        return true;
    }

    bool ReadFile(const OrderedJson& json, SoundFile& sound)
    {
        if (!json.is_string() || json.get_ref<const std::string&>().empty())
        {
            return Fail(Keys::File, "must be a file below the client folder");
        }
        sound.file = json.get<std::string>();
        if (const std::string problem = Paths::FindPathProblem(sound.file); !problem.empty())
        {
            return Fail(Keys::File, problem);
        }
        return true;
    }

    bool ReadChannels(const OrderedJson& json, SoundFile& sound)
    {
        const auto channels = json.find(Keys::Channels);
        if (channels == json.end())
        {
            return true;
        }
        long long count = 0;
        if (!Items::Json::ReadWholeNumber(*channels, count) || count < 1 || count > MaxSoundChannels)
        {
            return Fail(Keys::Channels, "must be a whole number from 1 to " + std::to_string(MaxSoundChannels));
        }
        sound.channels = static_cast<int>(count);
        return true;
    }

    bool ReadSpatial(const OrderedJson& json, SoundFile& sound)
    {
        const auto spatial = json.find(Keys::Spatial);
        if (spatial == json.end())
        {
            return true;
        }
        if (!spatial->is_boolean())
        {
            return Fail(Keys::Spatial, "must be true or false");
        }
        sound.spatial = spatial->get<bool>();
        return true;
    }

    bool ReadType(const OrderedJson& json, SoundFile& sound)
    {
        const auto type = json.find(Keys::Type);
        if (type == json.end())
        {
            return true;
        }
        if (!type->is_string())
        {
            return Fail(Keys::Type, "must be one of " + TypeList());
        }
        sound.type = FindSoundType(type->get_ref<const std::string&>());
        return sound.type.has_value() || Fail(Keys::Type, "must be one of " + TypeList());
    }

    bool WarnUnknown(const OrderedJson& json)
    {
        Items::Json::WarnAboutUnknownKeys(
            json, EntryKeys, SoundFileField(m_index, "") + ".",
            [&](const std::string& field, const std::string& message)
            { AddIssue(m_issues, ItemDataIssueSeverity::Warning, m_source, field, message); });
        return true;
    }

    size_t m_index;
    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

OrderedJson WriteEntry(const SoundFile& sound)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Id] = sound.id;
    if (!sound.name.empty())
    {
        json[Keys::Name] = sound.name;
    }
    json[Keys::File] = sound.file;
    json[Keys::Channels] = sound.channels;
    if (sound.spatial)
    {
        json[Keys::Spatial] = true;
    }
    if (sound.type)
    {
        json[Keys::Type] = std::string(SoundTypeWord(*sound.type));
    }
    return json;
}
} // namespace

std::string SoundFileField(size_t entry, std::string_view key)
{
    std::string field = std::string(Keys::Sounds) + "[" + std::to_string(entry) + "]";
    return key.empty() ? field : field + "." + std::string(key);
}

std::string_view SoundTypeWord(::Audio::SoundKind type)
{
    for (const auto& [kind, word] : TypeWords)
    {
        if (kind == type)
        {
            return word;
        }
    }
    return {};
}

std::optional<::Audio::SoundKind> FindSoundType(std::string_view word)
{
    for (const auto& [kind, typeWord] : TypeWords)
    {
        if (typeWord == word)
        {
            return kind;
        }
    }
    return std::nullopt;
}

void ReadSoundFilesJson(std::string_view text, const std::string& source, std::vector<SoundFile>& sounds,
                        std::vector<Items::ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, SoundFilesFormatVersion, root, issues))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, Keys::Sounds}, "",
                                      [&](const std::string& field, const std::string& message)
                                      { AddIssue(issues, ItemDataIssueSeverity::Warning, source, field, message); });
    const auto entries = root.find(Keys::Sounds);
    if (entries == root.end() || !entries->is_array())
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, source, Keys::Sounds, "missing or not a list of sounds");
        return;
    }
    for (size_t index = 0; index < entries->size(); ++index)
    {
        SoundFile sound;
        if (EntryReader(index, source, issues).Read((*entries)[index], sound))
        {
            sounds.push_back(std::move(sound));
        }
    }
}

std::string WriteSoundFilesJson(std::span<const SoundFile> sounds)
{
    OrderedJson entries = OrderedJson::array();
    for (const SoundFile& sound : sounds)
    {
        entries.push_back(WriteEntry(sound));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = SoundFilesFormatVersion;
    root[Keys::Sounds] = std::move(entries);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    return Items::Json::PutListEntriesOnOneLine(text, Keys::Sounds) + "\n";
}

void ValidateSoundFiles(std::span<const SoundFile> sounds, const std::string& source,
                        std::vector<Items::ItemDataIssue>& issues)
{
    for (size_t entry = 0; entry < sounds.size(); ++entry)
    {
        for (size_t other = 0; other < entry; ++other)
        {
            if (sounds[other].id == sounds[entry].id)
            {
                AddIssue(issues, ItemDataIssueSeverity::Fatal, source, SoundFileField(entry, Keys::Id),
                         sounds[entry].id + " is in sounds[" + std::to_string(other) + "] too: a sound loads one file");
                break;
            }
        }
    }
}

void ValidateSoundFilesOnDisk(std::span<const SoundFile> sounds, const std::string& source,
                              std::vector<Items::ItemDataIssue>& issues)
{
    for (size_t entry = 0; entry < sounds.size(); ++entry)
    {
        if (!Monsters::IsInClientFolder(sounds[entry].file))
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, source, SoundFileField(entry, Keys::File),
                     sounds[entry].file + " is not in the client folder: the sound is silent");
        }
    }
}
} // namespace Data::Sounds
