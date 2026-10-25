#include "stdafx.h"

#include "MonsterModelsJson.h"

#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <optional>
#include <set>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::KeySet;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* Models = "models";
constexpr const char* Slot = "slot";
constexpr const char* Effect = "effect";
constexpr const char* File = "file";
constexpr const char* TextureFolders = "textureFolders";
constexpr const char* WorldTextureFolders = "worldTextureFolders";
constexpr const char* Worlds = "worlds";
constexpr const char* Folders = "folders";
constexpr const char* TextureNames = "textureNames";
constexpr const char* Animations = "animations";
constexpr const char* HoldLastFrame = "holdLastFrame";
constexpr const char* HeadBone = "headBone";
constexpr const char* StreamMesh = "streamMesh";
constexpr const char* NoneBlendMeshes = "noneBlendMeshes";
constexpr const char* Sounds = "sounds";
constexpr const char* Id = "id";
constexpr const char* Channels = "channels";
constexpr const char* Spatial = "spatial";
constexpr const char* SoundSlots = "soundSlots";
constexpr const char* Idle = "idle";
constexpr const char* Attack = "attack";
constexpr const char* Die = "die";
constexpr const char* SecondSet = "secondSet";
constexpr const char* Bitmaps = "bitmaps";
constexpr const char* Filter = "filter";
constexpr const char* Wrap = "wrap";
constexpr const char* Parts = "parts";
constexpr const char* BoundingBox = "boundingBox";
constexpr const char* Min = "min";
constexpr const char* Max = "max";
constexpr const char* LinkBones = "linkBones";
constexpr const char* SoundEvents = "soundEvents";
constexpr const char* SoundEventWorlds = "soundEventWorlds";
constexpr const char* Frames = "frames";
constexpr const char* DeathEffectArmed = "deathEffectArmed";
constexpr const char* Chance = "chance";
constexpr const char* OneIn = "oneIn";
constexpr const char* Draw = "draw";
constexpr const char* Pick = "pick";
} // namespace Keys

constexpr const char* LinearFilter = "linear";
constexpr const char* NearestFilter = "nearest";
constexpr const char* RepeatWrap = "repeat";
constexpr const char* ClampWrap = "clamp";

const KeySet ModelKeys{Keys::Slot,           Keys::File,
                       Keys::TextureFolders, Keys::WorldTextureFolders,
                       Keys::TextureNames,   Keys::Animations,
                       Keys::HoldLastFrame,  Keys::HeadBone,
                       Keys::StreamMesh,     Keys::NoneBlendMeshes,
                       Keys::Sounds,         Keys::SoundSlots,
                       Keys::Bitmaps,        Keys::Parts,
                       Keys::BoundingBox,    Keys::LinkBones,
                       Keys::SoundEvents,    Keys::SoundEventWorlds};
const KeySet PartKeys{Keys::Slot,       Keys::Effect,        Keys::File,  Keys::TextureFolders,
                      Keys::Animations, Keys::HoldLastFrame, Keys::Worlds};
const KeySet SoundKeys{Keys::File, Keys::Id, Keys::Channels, Keys::Spatial};
const KeySet SoundSlotKeys{Keys::Idle, Keys::Attack, Keys::Die, Keys::SecondSet};
const KeySet SecondSetKeys{Keys::Idle, Keys::Attack, Keys::Die};
const KeySet BitmapKeys{Keys::File, Keys::Slot, Keys::Filter, Keys::Wrap};
const KeySet WorldFolderKeys{Keys::Worlds, Keys::Folders};
const KeySet BoxKeys{Keys::Min, Keys::Max};
const KeySet SoundEventKeys{Keys::Animations, Keys::Frames, Keys::DeathEffectArmed,
                            Keys::Chance,     Keys::Pick,   Keys::Sounds};
const KeySet ChanceKeys{Keys::OneIn, Keys::Draw};

constexpr const char* RandFpsDraw = "randFps";
constexpr const char* RandDraw = "rand";
// A sound event's chance: one in 1 to this.
constexpr int MaxOneIn = 100000;
// A pick chooses one of two sounds.
constexpr size_t PickSounds = 2;

constexpr std::string_view ModelFileExtension = ".bmd";
// A texture name fits Texture_t::FileName with its terminating zero.
constexpr size_t MaxTextureNameLength = 31;
constexpr int MaxMeshIndex = 49;
constexpr int MaxSoundChannels = 4;
constexpr int MaxLinkBones = 2;
// The idle, attack and die slots in a slot set.
constexpr size_t IdleSlot = 0;
constexpr size_t AttackSlot = 2;
constexpr size_t DieSlot = 4;

// The problems of one model: each names the model's slot.
class ModelIssues
{
public:
    ModelIssues(const std::string& source, std::string slot, std::vector<ItemDataIssue>& issues)
        : m_source(source), m_slot(std::move(slot)), m_issues(issues)
    {
    }

    void Error(const std::string& key, const std::string& message)
    {
        Add(ItemDataIssueSeverity::Error, key, message);
        m_valid = false;
    }

    void Warning(const std::string& key, const std::string& message)
    {
        Add(ItemDataIssueSeverity::Warning, key, message);
    }

    bool IsValid() const
    {
        return m_valid;
    }

    const std::string& Slot() const
    {
        return m_slot;
    }

private:
    void Add(ItemDataIssueSeverity severity, const std::string& key, const std::string& message)
    {
        m_issues.push_back({severity, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                            MonsterModelField(m_slot, key), message});
    }

    const std::string& m_source;
    std::string m_slot;
    std::vector<ItemDataIssue>& m_issues;
    bool m_valid = true;
};

std::optional<long long> ReadWhole(const OrderedJson& json)
{
    long long value = 0;
    if (!json.is_number_integer() || !Items::Json::ReadWholeNumber(json, value))
    {
        return std::nullopt;
    }
    return value;
}

std::optional<int> ReadWholeIn(const OrderedJson& json, long long low, long long high)
{
    const std::optional<long long> value = ReadWhole(json);
    if (!value || *value < low || *value > high)
    {
        return std::nullopt;
    }
    return static_cast<int>(*value);
}

std::string RangeText(long long low, long long high)
{
    return "must be a whole number from " + std::to_string(low) + " to " + std::to_string(high);
}

// A list of whole numbers from low to high, each once unless `repeats`;
// false on errors, which are reported.
bool ReadWholeList(const OrderedJson& json, const std::string& key, long long low, long long high,
                   std::vector<int>& values, ModelIssues& issues, bool repeats = false)
{
    if (!json.is_array())
    {
        issues.Error(key, "must be a list of whole numbers");
        return false;
    }
    std::set<int> seen;
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::optional<int> value = ReadWholeIn(json[i], low, high);
        if (!value)
        {
            issues.Error(key + "[" + std::to_string(i) + "]", RangeText(low, high));
            return false;
        }
        if (!seen.insert(*value).second && !repeats)
        {
            issues.Error(key, "has " + std::to_string(*value) + " more than once");
            return false;
        }
        values.push_back(*value);
    }
    return true;
}

bool ReadText(const OrderedJson& json, const std::string& key, std::string& text, ModelIssues& issues)
{
    if (!json.is_string() || json.get_ref<const std::string&>().empty())
    {
        issues.Error(key, "must be a text");
        return false;
    }
    text = json.get<std::string>();
    return true;
}

bool ReadPath(const OrderedJson& json, const std::string& key, std::string& path, ModelIssues& issues)
{
    if (!ReadText(json, key, path, issues))
    {
        return false;
    }
    const std::string problem = Paths::FindPathProblem(path);
    if (!problem.empty())
    {
        issues.Error(key, problem);
        return false;
    }
    return true;
}

bool EndsWithIgnoringCase(std::string_view text, std::string_view end)
{
    if (text.size() < end.size())
    {
        return false;
    }
    const std::string_view tail = text.substr(text.size() - end.size());
    return std::equal(tail.begin(), tail.end(), end.begin(),
                      [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == b; });
}

bool ReadModelFile(const OrderedJson& json, std::string& file, ModelIssues& issues)
{
    const auto value = json.find(Keys::File);
    if (value == json.end())
    {
        issues.Error(Keys::File, "missing");
        return false;
    }
    if (!ReadPath(*value, Keys::File, file, issues))
    {
        return false;
    }
    if (!EndsWithIgnoringCase(file, ModelFileExtension))
    {
        issues.Error(Keys::File, "must be a .bmd file");
        return false;
    }
    return true;
}

bool ReadFolders(const OrderedJson& json, const std::string& key, std::vector<std::string>& folders,
                 ModelIssues& issues)
{
    if (!json.is_array() || json.empty())
    {
        issues.Error(key, "must be a list of one folder or more");
        return false;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        std::string folder;
        if (!ReadPath(json[i], key + "[" + std::to_string(i) + "]", folder, issues))
        {
            return false;
        }
        folders.push_back(std::move(folder));
    }
    return true;
}

bool ReadSpeeds(const OrderedJson& json, std::vector<MonsterAnimationSpeed>& speeds, ModelIssues& issues)
{
    if (!json.is_object())
    {
        issues.Error(Keys::Animations, "must be an object of animation speeds");
        return false;
    }
    for (const auto& [name, value] : json.items())
    {
        const std::string key = std::string(Keys::Animations) + "." + name;
        const std::optional<int> animation = FindMonsterAnimation(name);
        if (!animation)
        {
            issues.Error(key, "is not an animation (stop1, stop2, walk, attack1, ... or a number from 12)");
            return false;
        }
        if (!value.is_number() || value.get<double>() < 0.0 || !std::isfinite(value.get<double>()))
        {
            issues.Error(key, "must be a number of 0 or more");
            return false;
        }
        speeds.emplace_back(*animation, static_cast<float>(value.get<double>()));
    }
    std::sort(speeds.begin(), speeds.end());
    return true;
}

bool ReadAnimationList(const OrderedJson& json, const std::string& key, std::vector<int>& animations,
                       ModelIssues& issues)
{
    if (!json.is_array())
    {
        issues.Error(key, "must be a list of animations");
        return false;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::optional<int> animation =
            json[i].is_string() ? FindMonsterAnimation(json[i].get<std::string>()) : std::nullopt;
        if (!animation)
        {
            issues.Error(key + "[" + std::to_string(i) + "]", "is not an animation");
            return false;
        }
        if (std::find(animations.begin(), animations.end(), *animation) != animations.end())
        {
            issues.Error(key, "has " + GetMonsterAnimationName(*animation) + " more than once");
            return false;
        }
        animations.push_back(*animation);
    }
    std::sort(animations.begin(), animations.end());
    return true;
}

// The fields a model and a part both have.
void ReadLoad(const OrderedJson& json, MonsterModelLoad& load, ModelIssues& issues)
{
    ReadModelFile(json, load.file, issues);
    const auto folders = json.find(Keys::TextureFolders);
    if (folders == json.end())
    {
        issues.Error(Keys::TextureFolders, "missing");
    }
    else
    {
        ReadFolders(*folders, Keys::TextureFolders, load.textureFolders, issues);
    }
    if (const auto speeds = json.find(Keys::Animations); speeds != json.end())
    {
        ReadSpeeds(*speeds, load.speeds, issues);
    }
    if (const auto hold = json.find(Keys::HoldLastFrame); hold != json.end())
    {
        std::vector<int> animations;
        if (ReadAnimationList(*hold, Keys::HoldLastFrame, animations, issues))
        {
            load.holdLastFrame = std::move(animations);
        }
    }
}

void ReadWorldFolders(const OrderedJson& json, std::vector<MonsterModelWorldFolders>& worldFolders, ModelIssues& issues)
{
    if (!json.is_array())
    {
        issues.Error(Keys::WorldTextureFolders, "must be a list");
        return;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::string key = std::string(Keys::WorldTextureFolders) + "[" + std::to_string(i) + "]";
        const OrderedJson& entry = json[i];
        if (!entry.is_object() || !entry.contains(Keys::Worlds) || !entry.contains(Keys::Folders))
        {
            issues.Error(key, "must be an object with worlds and folders");
            return;
        }
        MonsterModelWorldFolders folders;
        if (!ReadWholeList(entry[Keys::Worlds], key + "." + Keys::Worlds, 0, std::numeric_limits<int>::max(),
                           folders.worlds, issues) ||
            !ReadFolders(entry[Keys::Folders], key + "." + Keys::Folders, folders.folders, issues))
        {
            return;
        }
        Items::Json::WarnAboutUnknownKeys(entry, WorldFolderKeys, key + ".",
                                          [&](const std::string& unknown, const std::string& message)
                                          { issues.Warning(unknown, message); });
        worldFolders.push_back(std::move(folders));
    }
}

void ReadTextureNames(const OrderedJson& json, std::vector<std::pair<int, std::string>>& names, ModelIssues& issues)
{
    if (!json.is_object())
    {
        issues.Error(Keys::TextureNames, "must be an object of texture names by mesh");
        return;
    }
    for (const auto& [mesh, value] : json.items())
    {
        const std::string key = std::string(Keys::TextureNames) + "." + mesh;
        int number = -1;
        const auto [end, error] = std::from_chars(mesh.data(), mesh.data() + mesh.size(), number);
        if (error != std::errc() || end != mesh.data() + mesh.size() || number < 0 || number > MaxMeshIndex)
        {
            issues.Error(key, "the mesh must be a number from 0 to " + std::to_string(MaxMeshIndex));
            return;
        }
        std::string name;
        if (!ReadText(value, key, name, issues))
        {
            return;
        }
        if (name.size() > MaxTextureNameLength)
        {
            issues.Error(key, "must have at most " + std::to_string(MaxTextureNameLength) + " characters");
            return;
        }
        names.emplace_back(number, std::move(name));
    }
    std::sort(names.begin(), names.end());
}

void ReadSounds(const OrderedJson& json, std::vector<MonsterModelSound>& sounds, ModelIssues& issues)
{
    if (!json.is_array())
    {
        issues.Error(Keys::Sounds, "must be a list of sounds");
        return;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::string key = std::string(Keys::Sounds) + "[" + std::to_string(i) + "]";
        const OrderedJson& entry = json[i];
        if (!entry.is_object())
        {
            issues.Error(key, "must be an object");
            return;
        }
        MonsterModelSound sound;
        const auto file = entry.find(Keys::File);
        if (file == entry.end())
        {
            issues.Error(key + "." + Keys::File, "missing");
            return;
        }
        if (!ReadPath(*file, key + "." + Keys::File, sound.file, issues))
        {
            return;
        }
        if (const auto id = entry.find(Keys::Id);
            id != entry.end() && !ReadText(*id, key + "." + Keys::Id, sound.id, issues))
        {
            return;
        }
        if (const auto channels = entry.find(Keys::Channels); channels != entry.end())
        {
            const std::optional<int> count = ReadWholeIn(*channels, 1, MaxSoundChannels);
            if (!count)
            {
                issues.Error(key + "." + Keys::Channels, RangeText(1, MaxSoundChannels));
                return;
            }
            sound.channels = *count;
        }
        if (const auto spatial = entry.find(Keys::Spatial); spatial != entry.end())
        {
            if (!spatial->is_boolean())
            {
                issues.Error(key + "." + Keys::Spatial, "must be true or false");
                return;
            }
            sound.spatial = spatial->get<bool>();
        }
        Items::Json::WarnAboutUnknownKeys(entry, SoundKeys, key + ".",
                                          [&](const std::string& unknown, const std::string& message)
                                          { issues.Warning(unknown, message); });
        sounds.push_back(std::move(sound));
    }
}

// A slot names a sound, or is null for no sound.
bool ReadSlotValue(const OrderedJson& json, const std::string& key, std::string& value, ModelIssues& issues)
{
    if (json.is_null())
    {
        value.clear();
        return true;
    }
    return ReadText(json, key, value, issues);
}

bool ReadSlotPair(const OrderedJson& json, const std::string& key, MonsterSoundSlotSet& slots, size_t first,
                  ModelIssues& issues)
{
    if (!json.is_array() || json.size() != 2)
    {
        issues.Error(key, "must be a list of two sounds");
        return false;
    }
    return ReadSlotValue(json[0], key + "[0]", slots[first], issues) &&
           ReadSlotValue(json[1], key + "[1]", slots[first + 1], issues);
}

bool ReadSlotSet(const OrderedJson& json, const std::string& key, const KeySet& keys, MonsterSoundSlotSet& slots,
                 ModelIssues& issues)
{
    if (!json.is_object() || !json.contains(Keys::Idle) || !json.contains(Keys::Attack) || !json.contains(Keys::Die))
    {
        issues.Error(key, "must be an object with idle, attack and die");
        return false;
    }
    const std::string prefix = key + ".";
    if (!ReadSlotPair(json[Keys::Idle], prefix + Keys::Idle, slots, IdleSlot, issues) ||
        !ReadSlotPair(json[Keys::Attack], prefix + Keys::Attack, slots, AttackSlot, issues) ||
        !ReadSlotValue(json[Keys::Die], prefix + Keys::Die, slots[DieSlot], issues))
    {
        return false;
    }
    Items::Json::WarnAboutUnknownKeys(json, keys, prefix, [&](const std::string& unknown, const std::string& message)
                                      { issues.Warning(unknown, message); });
    return true;
}

void ReadSoundSlots(const OrderedJson& json, std::optional<MonsterSoundSlots>& soundSlots, ModelIssues& issues)
{
    MonsterSoundSlots slots;
    if (!ReadSlotSet(json, Keys::SoundSlots, SoundSlotKeys, slots.first, issues))
    {
        return;
    }
    if (const auto second = json.find(Keys::SecondSet); second != json.end())
    {
        MonsterSoundSlotSet set;
        if (!ReadSlotSet(*second, std::string(Keys::SoundSlots) + "." + Keys::SecondSet, SecondSetKeys, set, issues))
        {
            return;
        }
        slots.second = std::move(set);
    }
    soundSlots = std::move(slots);
}

std::optional<MonsterSoundDraw> FindSoundDraw(const OrderedJson& json)
{
    if (json.is_string() && json.get_ref<const std::string&>() == RandFpsDraw)
    {
        return MonsterSoundDraw::RandFps;
    }
    if (json.is_string() && json.get_ref<const std::string&>() == RandDraw)
    {
        return MonsterSoundDraw::Rand;
    }
    return std::nullopt;
}

bool ReadSoundDraw(const OrderedJson& json, const std::string& key, MonsterSoundDraw& draw, ModelIssues& issues)
{
    const std::optional<MonsterSoundDraw> found = FindSoundDraw(json);
    if (!found)
    {
        issues.Error(key, std::string("must be ") + RandFpsDraw + " or " + RandDraw);
        return false;
    }
    draw = *found;
    return true;
}

bool ReadFrames(const OrderedJson& json, const std::string& key, std::pair<float, float>& frames, ModelIssues& issues)
{
    const auto isFrame = [](const OrderedJson& value)
    { return value.is_number() && std::isfinite(value.get<double>()) && value.get<double>() >= 0.0; };
    if (!json.is_array() || json.size() != 2 || !isFrame(json[0]) || !isFrame(json[1]) ||
        json[0].get<double>() >= json[1].get<double>())
    {
        issues.Error(key, "must be two frames, the first below the second: [7, 8]");
        return false;
    }
    frames = {static_cast<float>(json[0].get<double>()), static_cast<float>(json[1].get<double>())};
    return true;
}

bool ReadChance(const OrderedJson& json, const std::string& key, MonsterSoundChance& chance, ModelIssues& issues)
{
    if (!json.is_object() || !json.contains(Keys::OneIn) || !json.contains(Keys::Draw))
    {
        issues.Error(key, "must be an object with oneIn and draw");
        return false;
    }
    const std::optional<int> oneIn = ReadWholeIn(json[Keys::OneIn], 1, MaxOneIn);
    if (!oneIn)
    {
        issues.Error(key + "." + Keys::OneIn, RangeText(1, MaxOneIn));
        return false;
    }
    chance.oneIn = *oneIn;
    if (!ReadSoundDraw(json[Keys::Draw], key + "." + Keys::Draw, chance.draw, issues))
    {
        return false;
    }
    Items::Json::WarnAboutUnknownKeys(json, ChanceKeys, key + ".",
                                      [&](const std::string& unknown, const std::string& message)
                                      { issues.Warning(unknown, message); });
    return true;
}

bool ReadEventSounds(const OrderedJson& json, const std::string& key, std::vector<std::string>& sounds,
                     ModelIssues& issues)
{
    if (!json.is_array() || json.empty())
    {
        issues.Error(key, "must be a list of one sound or more");
        return false;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        std::string sound;
        if (!ReadText(json[i], key + "[" + std::to_string(i) + "]", sound, issues))
        {
            return false;
        }
        sounds.push_back(std::move(sound));
    }
    return true;
}

bool ReadSoundEvent(const OrderedJson& entry, const std::string& key, MonsterSoundEvent& event, ModelIssues& issues)
{
    if (!entry.is_object() || !entry.contains(Keys::Animations) || !entry.contains(Keys::Sounds))
    {
        issues.Error(key, "must be an object with animations and sounds");
        return false;
    }
    const std::string prefix = key + ".";
    if (!ReadAnimationList(entry[Keys::Animations], prefix + Keys::Animations, event.animations, issues))
    {
        return false;
    }
    if (event.animations.empty())
    {
        issues.Error(prefix + Keys::Animations, "must name one animation or more");
        return false;
    }
    if (const auto frames = entry.find(Keys::Frames); frames != entry.end())
    {
        std::pair<float, float> range;
        if (!ReadFrames(*frames, prefix + Keys::Frames, range, issues))
        {
            return false;
        }
        event.frames = range;
    }
    if (const auto armed = entry.find(Keys::DeathEffectArmed); armed != entry.end())
    {
        if (!armed->is_boolean())
        {
            issues.Error(prefix + Keys::DeathEffectArmed, "must be true or false");
            return false;
        }
        event.deathEffectArmed = armed->get<bool>();
    }
    if (const auto chance = entry.find(Keys::Chance); chance != entry.end())
    {
        MonsterSoundChance read;
        if (!ReadChance(*chance, prefix + Keys::Chance, read, issues))
        {
            return false;
        }
        event.chance = read;
    }
    if (const auto pick = entry.find(Keys::Pick); pick != entry.end())
    {
        MonsterSoundDraw draw = MonsterSoundDraw::RandFps;
        if (!ReadSoundDraw(*pick, prefix + Keys::Pick, draw, issues))
        {
            return false;
        }
        event.pick = draw;
    }
    if (!ReadEventSounds(entry[Keys::Sounds], prefix + Keys::Sounds, event.sounds, issues))
    {
        return false;
    }
    if (event.pick && event.sounds.size() != PickSounds)
    {
        issues.Error(prefix + Keys::Sounds, "must be two sounds with pick");
        return false;
    }
    Items::Json::WarnAboutUnknownKeys(entry, SoundEventKeys, prefix,
                                      [&](const std::string& unknown, const std::string& message)
                                      { issues.Warning(unknown, message); });
    return true;
}

void ReadSoundEvents(const OrderedJson& json, std::vector<MonsterSoundEvent>& events, ModelIssues& issues)
{
    if (!json.is_array())
    {
        issues.Error(Keys::SoundEvents, "must be a list of sound events");
        return;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        MonsterSoundEvent event;
        if (!ReadSoundEvent(json[i], std::string(Keys::SoundEvents) + "[" + std::to_string(i) + "]", event, issues))
        {
            return;
        }
        events.push_back(std::move(event));
    }
}

bool ReadChoice(const OrderedJson& entry, const char* key, const std::string& field, const char* yes, const char* no,
                bool& value, ModelIssues& issues)
{
    const auto choice = entry.find(key);
    if (choice == entry.end())
    {
        value = false;
        return true;
    }
    if (choice->is_string() && choice->get_ref<const std::string&>() == yes)
    {
        value = true;
        return true;
    }
    if (choice->is_string() && choice->get_ref<const std::string&>() == no)
    {
        value = false;
        return true;
    }
    issues.Error(field + "." + key, std::string("must be ") + yes + " or " + no);
    return false;
}

void ReadBitmaps(const OrderedJson& json, std::vector<MonsterModelBitmap>& bitmaps, ModelIssues& issues)
{
    if (!json.is_array())
    {
        issues.Error(Keys::Bitmaps, "must be a list of bitmaps");
        return;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::string key = std::string(Keys::Bitmaps) + "[" + std::to_string(i) + "]";
        const OrderedJson& entry = json[i];
        if (!entry.is_object() || !entry.contains(Keys::File) || !entry.contains(Keys::Slot))
        {
            issues.Error(key, "must be an object with file and slot");
            return;
        }
        MonsterModelBitmap bitmap;
        if (!ReadPath(entry[Keys::File], key + "." + Keys::File, bitmap.file, issues) ||
            !ReadText(entry[Keys::Slot], key + "." + Keys::Slot, bitmap.slot, issues) ||
            !ReadChoice(entry, Keys::Filter, key, LinearFilter, NearestFilter, bitmap.linearFilter, issues) ||
            !ReadChoice(entry, Keys::Wrap, key, RepeatWrap, ClampWrap, bitmap.repeat, issues))
        {
            return;
        }
        Items::Json::WarnAboutUnknownKeys(entry, BitmapKeys, key + ".",
                                          [&](const std::string& unknown, const std::string& message)
                                          { issues.Warning(unknown, message); });
        bitmaps.push_back(std::move(bitmap));
    }
}

void ReadParts(const OrderedJson& json, std::vector<MonsterModelPart>& parts, ModelIssues& issues)
{
    if (!json.is_array())
    {
        issues.Error(Keys::Parts, "must be a list of part models");
        return;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::string key = std::string(Keys::Parts) + "[" + std::to_string(i) + "]";
        const OrderedJson& entry = json[i];
        if (!entry.is_object())
        {
            issues.Error(key, "must be an object");
            return;
        }
        MonsterModelPart part;
        const bool hasSlot = entry.contains(Keys::Slot);
        const bool hasEffect = entry.contains(Keys::Effect);
        if (hasSlot == hasEffect)
        {
            issues.Error(key, "must have a slot or an effect, not both");
            return;
        }
        if ((hasSlot && !ReadText(entry[Keys::Slot], key + "." + Keys::Slot, part.slot, issues)) ||
            (hasEffect && !ReadText(entry[Keys::Effect], key + "." + Keys::Effect, part.effect, issues)))
        {
            return;
        }
        ReadLoad(entry, part.load, issues);
        if (const auto worlds = entry.find(Keys::Worlds); worlds != entry.end())
        {
            ReadWholeList(*worlds, key + "." + Keys::Worlds, 0, std::numeric_limits<int>::max(), part.worlds, issues);
        }
        Items::Json::WarnAboutUnknownKeys(entry, PartKeys, key + ".",
                                          [&](const std::string& unknown, const std::string& message)
                                          { issues.Warning(unknown, message); });
        parts.push_back(std::move(part));
    }
}

bool ReadPoint(const OrderedJson& json, const std::string& key, std::array<float, 3>& point, ModelIssues& issues)
{
    if (!json.is_array() || json.size() != point.size() ||
        !std::all_of(json.begin(), json.end(), [](const OrderedJson& value) { return value.is_number(); }))
    {
        issues.Error(key, "must be a list of three numbers");
        return false;
    }
    for (size_t i = 0; i < point.size(); ++i)
    {
        point[i] = static_cast<float>(json[i].get<double>());
    }
    return true;
}

void ReadBox(const OrderedJson& json, MonsterModelBody& body, ModelIssues& issues)
{
    if (!json.is_object())
    {
        issues.Error(Keys::BoundingBox, "must be an object with min and max");
        return;
    }
    const std::string prefix = std::string(Keys::BoundingBox) + ".";
    std::array<float, 3> point{};
    if (const auto min = json.find(Keys::Min); min != json.end() && ReadPoint(*min, prefix + Keys::Min, point, issues))
    {
        body.boxMin = point;
    }
    if (const auto max = json.find(Keys::Max); max != json.end() && ReadPoint(*max, prefix + Keys::Max, point, issues))
    {
        body.boxMax = point;
    }
    Items::Json::WarnAboutUnknownKeys(json, BoxKeys, prefix, [&](const std::string& unknown, const std::string& message)
                                      { issues.Warning(unknown, message); });
}

void ReadLinkBones(const OrderedJson& json, MonsterModelBody& body, ModelIssues& issues)
{
    if (!json.is_array() || json.size() > MaxLinkBones)
    {
        issues.Error(Keys::LinkBones, "must be a list of at most two bones");
        return;
    }
    std::vector<int> linkBones;
    // Both hands may hang on one bone.
    constexpr bool RepeatsAllowed = true;
    if (ReadWholeList(json, Keys::LinkBones, 0, std::numeric_limits<unsigned char>::max(), linkBones, issues,
                      RepeatsAllowed))
    {
        body.linkBones = std::move(linkBones);
    }
}

void ReadOptionalWhole(const OrderedJson& json, const char* key, long long low, long long high,
                       std::optional<int>& value, ModelIssues& issues)
{
    const auto field = json.find(key);
    if (field == json.end())
    {
        return;
    }
    const std::optional<int> number = ReadWholeIn(*field, low, high);
    if (!number)
    {
        issues.Error(key, RangeText(low, high));
        return;
    }
    value = number;
}

void ReadModelFields(const OrderedJson& json, MonsterModelDefinition& model, ModelIssues& issues)
{
    ReadLoad(json, model.load, issues);
    if (const auto folders = json.find(Keys::WorldTextureFolders); folders != json.end())
    {
        ReadWorldFolders(*folders, model.worldTextureFolders, issues);
    }
    if (const auto names = json.find(Keys::TextureNames); names != json.end())
    {
        ReadTextureNames(*names, model.textureNames, issues);
    }
    ReadOptionalWhole(json, Keys::HeadBone, -1, std::numeric_limits<int>::max(), model.headBone, issues);
    ReadOptionalWhole(json, Keys::StreamMesh, std::numeric_limits<char>::min(), std::numeric_limits<char>::max(),
                      model.streamMesh, issues);
    if (const auto meshes = json.find(Keys::NoneBlendMeshes); meshes != json.end())
    {
        ReadWholeList(*meshes, Keys::NoneBlendMeshes, 0, MaxMeshIndex, model.noneBlendMeshes, issues);
    }
    if (const auto sounds = json.find(Keys::Sounds); sounds != json.end())
    {
        ReadSounds(*sounds, model.sounds, issues);
    }
    if (const auto slots = json.find(Keys::SoundSlots); slots != json.end())
    {
        ReadSoundSlots(*slots, model.soundSlots, issues);
    }
    if (const auto events = json.find(Keys::SoundEvents); events != json.end())
    {
        ReadSoundEvents(*events, model.soundEvents, issues);
    }
    if (const auto worlds = json.find(Keys::SoundEventWorlds); worlds != json.end())
    {
        ReadWholeList(*worlds, Keys::SoundEventWorlds, 0, std::numeric_limits<int>::max(), model.soundEventWorlds,
                      issues);
        std::sort(model.soundEventWorlds.begin(), model.soundEventWorlds.end());
    }
    if (const auto bitmaps = json.find(Keys::Bitmaps); bitmaps != json.end())
    {
        ReadBitmaps(*bitmaps, model.bitmaps, issues);
    }
    if (const auto parts = json.find(Keys::Parts); parts != json.end())
    {
        ReadParts(*parts, model.parts, issues);
    }
    if (const auto box = json.find(Keys::BoundingBox); box != json.end())
    {
        ReadBox(*box, model.body, issues);
    }
    if (const auto bones = json.find(Keys::LinkBones); bones != json.end())
    {
        ReadLinkBones(*bones, model.body, issues);
    }
}

std::optional<MonsterModelDefinition> ReadModel(const OrderedJson& json, size_t index, const std::string& source,
                                                std::vector<ItemDataIssue>& issues)
{
    const std::string entry = std::string(Keys::Models) + "[" + std::to_string(index) + "]";
    if (!json.is_object())
    {
        issues.push_back({ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, entry,
                          "must be an object"});
        return std::nullopt;
    }
    const auto slot = json.find(Keys::Slot);
    if (slot == json.end() || !slot->is_string() || !FindMonsterModelSlot(slot->get<std::string>()))
    {
        issues.push_back({ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                          entry + "." + Keys::Slot,
                          "missing or not a monster model type (MONSTER_MODEL_...) or an NPC model slot"});
        return std::nullopt;
    }
    MonsterModelDefinition model;
    model.slot = slot->get<std::string>();
    ModelIssues modelIssues(source, model.slot, issues);
    ReadModelFields(json, model, modelIssues);
    Items::Json::WarnAboutUnknownKeys(json, ModelKeys, "", [&](const std::string& unknown, const std::string& message)
                                      { modelIssues.Warning(unknown, message); });
    if (!modelIssues.IsValid())
    {
        return std::nullopt;
    }
    return model;
}

// ---------------------------------------------------------------- writing

// 0.7 instead of 0.69999998807907104: the shortest text that reads back as
// the same float, found digit by digit (std::to_chars of a float, which
// does it at once, is missing before macOS 13.3); 270 instead of 270.0.
OrderedJson WriteFloat(float value)
{
    if (value == std::floor(value) && std::abs(value) < static_cast<float>(1 << 24))
    {
        return static_cast<long long>(value);
    }
    std::array<char, 32> buffer{};
    for (int digits = 1; digits <= std::numeric_limits<float>::max_digits10; ++digits)
    {
        std::snprintf(buffer.data(), buffer.size(), "%.*g", digits, static_cast<double>(value));
        if (std::strtof(buffer.data(), nullptr) == value)
        {
            break;
        }
    }
    return std::strtod(buffer.data(), nullptr);
}

OrderedJson WriteSpeeds(const std::vector<MonsterAnimationSpeed>& speeds)
{
    OrderedJson json = OrderedJson::object();
    for (const auto& [animation, speed] : speeds)
    {
        json[GetMonsterAnimationName(animation)] = WriteFloat(speed);
    }
    return json;
}

OrderedJson WriteAnimationList(const std::vector<int>& animations)
{
    OrderedJson json = OrderedJson::array();
    for (const int animation : animations)
    {
        json.push_back(GetMonsterAnimationName(animation));
    }
    return json;
}

void WriteLoad(const MonsterModelLoad& load, OrderedJson& json)
{
    json[Keys::File] = load.file;
    json[Keys::TextureFolders] = load.textureFolders;
}

void WriteAnimations(const MonsterModelLoad& load, OrderedJson& json)
{
    if (!load.speeds.empty())
    {
        json[Keys::Animations] = WriteSpeeds(load.speeds);
    }
    if (load.holdLastFrame)
    {
        json[Keys::HoldLastFrame] = WriteAnimationList(*load.holdLastFrame);
    }
}

OrderedJson WriteSlotValue(const std::string& value)
{
    return value.empty() ? OrderedJson(nullptr) : OrderedJson(value);
}

OrderedJson WriteSlotSet(const MonsterSoundSlotSet& slots)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Idle] = OrderedJson::array({WriteSlotValue(slots[IdleSlot]), WriteSlotValue(slots[IdleSlot + 1])});
    json[Keys::Attack] = OrderedJson::array({WriteSlotValue(slots[AttackSlot]), WriteSlotValue(slots[AttackSlot + 1])});
    json[Keys::Die] = WriteSlotValue(slots[DieSlot]);
    return json;
}

OrderedJson WriteSound(const MonsterModelSound& sound)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::File] = sound.file;
    if (!sound.id.empty())
    {
        json[Keys::Id] = sound.id;
    }
    json[Keys::Channels] = sound.channels;
    json[Keys::Spatial] = sound.spatial;
    return json;
}

OrderedJson WriteBitmap(const MonsterModelBitmap& bitmap)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::File] = bitmap.file;
    json[Keys::Slot] = bitmap.slot;
    json[Keys::Filter] = bitmap.linearFilter ? LinearFilter : NearestFilter;
    json[Keys::Wrap] = bitmap.repeat ? RepeatWrap : ClampWrap;
    return json;
}

OrderedJson WritePart(const MonsterModelPart& part)
{
    OrderedJson json = OrderedJson::object();
    if (!part.slot.empty())
    {
        json[Keys::Slot] = part.slot;
    }
    else
    {
        json[Keys::Effect] = part.effect;
    }
    WriteLoad(part.load, json);
    WriteAnimations(part.load, json);
    if (!part.worlds.empty())
    {
        json[Keys::Worlds] = part.worlds;
    }
    return json;
}

OrderedJson WritePoint(const std::array<float, 3>& point)
{
    OrderedJson json = OrderedJson::array();
    for (const float value : point)
    {
        json.push_back(WriteFloat(value));
    }
    return json;
}

void WriteBody(const MonsterModelBody& body, OrderedJson& json)
{
    if (body.boxMin || body.boxMax)
    {
        OrderedJson box = OrderedJson::object();
        if (body.boxMin)
        {
            box[Keys::Min] = WritePoint(*body.boxMin);
        }
        if (body.boxMax)
        {
            box[Keys::Max] = WritePoint(*body.boxMax);
        }
        json[Keys::BoundingBox] = std::move(box);
    }
    if (body.linkBones)
    {
        json[Keys::LinkBones] = *body.linkBones;
    }
}

OrderedJson WriteSoundDraw(MonsterSoundDraw draw)
{
    return draw == MonsterSoundDraw::Rand ? RandDraw : RandFpsDraw;
}

OrderedJson WriteSoundEvent(const MonsterSoundEvent& event)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Animations] = WriteAnimationList(event.animations);
    if (event.frames)
    {
        json[Keys::Frames] = OrderedJson::array({WriteFloat(event.frames->first), WriteFloat(event.frames->second)});
    }
    if (event.deathEffectArmed)
    {
        json[Keys::DeathEffectArmed] = true;
    }
    if (event.chance)
    {
        OrderedJson chance = OrderedJson::object();
        chance[Keys::OneIn] = event.chance->oneIn;
        chance[Keys::Draw] = WriteSoundDraw(event.chance->draw);
        json[Keys::Chance] = std::move(chance);
    }
    if (event.pick)
    {
        json[Keys::Pick] = WriteSoundDraw(*event.pick);
    }
    json[Keys::Sounds] = event.sounds;
    return json;
}

void WriteSoundFields(const MonsterModelDefinition& model, OrderedJson& json)
{
    if (!model.sounds.empty())
    {
        OrderedJson sounds = OrderedJson::array();
        for (const MonsterModelSound& sound : model.sounds)
        {
            sounds.push_back(WriteSound(sound));
        }
        json[Keys::Sounds] = std::move(sounds);
    }
    if (model.soundSlots)
    {
        OrderedJson slots = WriteSlotSet(model.soundSlots->first);
        if (model.soundSlots->second)
        {
            slots[Keys::SecondSet] = WriteSlotSet(*model.soundSlots->second);
        }
        json[Keys::SoundSlots] = std::move(slots);
    }
    if (!model.soundEvents.empty())
    {
        OrderedJson events = OrderedJson::array();
        for (const MonsterSoundEvent& event : model.soundEvents)
        {
            events.push_back(WriteSoundEvent(event));
        }
        json[Keys::SoundEvents] = std::move(events);
    }
    if (!model.soundEventWorlds.empty())
    {
        json[Keys::SoundEventWorlds] = model.soundEventWorlds;
    }
}

OrderedJson WriteModel(const MonsterModelDefinition& model)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Slot] = model.slot;
    WriteLoad(model.load, json);
    if (!model.worldTextureFolders.empty())
    {
        OrderedJson folders = OrderedJson::array();
        for (const MonsterModelWorldFolders& entry : model.worldTextureFolders)
        {
            OrderedJson folder = OrderedJson::object();
            folder[Keys::Worlds] = entry.worlds;
            folder[Keys::Folders] = entry.folders;
            folders.push_back(std::move(folder));
        }
        json[Keys::WorldTextureFolders] = std::move(folders);
    }
    if (!model.textureNames.empty())
    {
        OrderedJson names = OrderedJson::object();
        for (const auto& [mesh, name] : model.textureNames)
        {
            names[std::to_string(mesh)] = name;
        }
        json[Keys::TextureNames] = std::move(names);
    }
    WriteAnimations(model.load, json);
    if (model.headBone)
    {
        json[Keys::HeadBone] = *model.headBone;
    }
    if (model.streamMesh)
    {
        json[Keys::StreamMesh] = *model.streamMesh;
    }
    if (!model.noneBlendMeshes.empty())
    {
        json[Keys::NoneBlendMeshes] = model.noneBlendMeshes;
    }
    WriteSoundFields(model, json);
    if (!model.bitmaps.empty())
    {
        OrderedJson bitmaps = OrderedJson::array();
        for (const MonsterModelBitmap& bitmap : model.bitmaps)
        {
            bitmaps.push_back(WriteBitmap(bitmap));
        }
        json[Keys::Bitmaps] = std::move(bitmaps);
    }
    if (!model.parts.empty())
    {
        OrderedJson parts = OrderedJson::array();
        for (const MonsterModelPart& part : model.parts)
        {
            parts.push_back(WritePart(part));
        }
        json[Keys::Parts] = std::move(parts);
    }
    WriteBody(model.body, json);
    return json;
}

// Short lists and objects on one line, each sound and bitmap on its own line.
std::string FormatText(std::string text)
{
    using namespace Items::Json;
    for (const char* key : {Keys::TextureFolders, Keys::HoldLastFrame, Keys::NoneBlendMeshes, Keys::Worlds,
                            Keys::Folders, Keys::Idle, Keys::Attack, Keys::LinkBones, Keys::SoundEventWorlds})
    {
        text = PutListsOnOneLine(text, key);
    }
    for (const char* key : {Keys::Animations, Keys::TextureNames, Keys::BoundingBox})
    {
        text = PutObjectsOnOneLine(text, key);
    }
    for (const char* key : {Keys::Sounds, Keys::Bitmaps, Keys::WorldTextureFolders, Keys::SoundEvents})
    {
        text = PutListEntriesOnOneLine(text, key);
    }
    return text;
}

int SortSlot(const MonsterModelDefinition& model)
{
    return FindMonsterModelSlot(model.slot).value_or(std::numeric_limits<int>::max());
}
} // namespace

std::string MonsterModelField(std::string_view slot, std::string_view key)
{
    return "model " + std::string(slot) + " " + std::string(key);
}

void ReadMonsterModelsJson(std::string_view text, const std::string& source,
                           std::vector<MonsterModelDefinition>& models, std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, MonsterModelsFormatVersion, root, issues))
    {
        return;
    }
    const auto addFileIssue = [&](ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    { issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message}); };
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, Keys::Models}, "",
                                      [&](const std::string& unknown, const std::string& message)
                                      { addFileIssue(ItemDataIssueSeverity::Warning, unknown, message); });
    const auto entries = root.find(Keys::Models);
    if (entries == root.end() || !entries->is_array())
    {
        addFileIssue(ItemDataIssueSeverity::Error, Keys::Models, "missing or not a list of models");
        return;
    }
    std::set<int> slots;
    for (size_t index = 0; index < entries->size(); ++index)
    {
        std::optional<MonsterModelDefinition> model = ReadModel((*entries)[index], index, source, issues);
        if (!model)
        {
            continue;
        }
        if (!slots.insert(*FindMonsterModelSlot(model->slot)).second)
        {
            addFileIssue(ItemDataIssueSeverity::Error, MonsterModelField(model->slot, Keys::Slot),
                         "is in the list more than once");
            continue;
        }
        models.push_back(std::move(*model));
    }
}

std::string WriteMonsterModelsJson(std::span<const MonsterModelDefinition> models)
{
    std::vector<const MonsterModelDefinition*> sorted;
    sorted.reserve(models.size());
    for (const MonsterModelDefinition& model : models)
    {
        sorted.push_back(&model);
    }
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const MonsterModelDefinition* left, const MonsterModelDefinition* right)
                     { return SortSlot(*left) < SortSlot(*right); });

    OrderedJson entries = OrderedJson::array();
    for (const MonsterModelDefinition* model : sorted)
    {
        entries.push_back(WriteModel(*model));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = MonsterModelsFormatVersion;
    root[Keys::Models] = std::move(entries);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    return FormatText(text) + "\n";
}
} // namespace Data::Monsters
