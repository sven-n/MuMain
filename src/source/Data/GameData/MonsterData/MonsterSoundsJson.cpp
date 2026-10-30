#include "stdafx.h"

#include "MonsterSoundsJson.h"

#include "Data/GameData/MonsterData/MonsterJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterSoundMoments.h"

#include <algorithm>
#include <optional>
#include <set>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Json::EntryIssues;
using Json::KeySet;
using Json::OrderedJson;

namespace Keys
{
constexpr const char* SoundSets = "soundSets";
constexpr const char* Name = "name";
constexpr const char* Sounds = "sounds";
constexpr const char* File = "file";
constexpr const char* Id = "id";
constexpr const char* Channels = "channels";
constexpr const char* Spatial = "spatial";
constexpr const char* SoundEvents = "soundEvents";
constexpr const char* Maps = "maps";
constexpr const char* Animations = "animations";
constexpr const char* On = "on";
constexpr const char* Cues = "cues";
constexpr const char* DeathEffectArmed = "deathEffectArmed";
constexpr const char* HeardAnywhere = "heardAnywhere";
constexpr const char* AtMonster = "atMonster";
constexpr const char* Chance = "chance";
constexpr const char* OneIn = "oneIn";
constexpr const char* Draw = "draw";
constexpr const char* Pick = "pick";
// The fields of the model events before the sound sets (Q-MN78, Q-MN79).
constexpr const char* OldFrames = "frames";
constexpr const char* OldSubType = "subType";
constexpr const char* OldNotSubType = "notSubType";
} // namespace Keys

const KeySet SetKeys{Keys::Name, Keys::Sounds, Keys::SoundEvents, Keys::Maps};
const KeySet SoundKeys{Keys::File, Keys::Id, Keys::Channels, Keys::Spatial};
// With the old keys: the reader refuses them, they are not unknown.
const KeySet SoundEventKeys{Keys::Animations,    Keys::On,        Keys::Cues,       Keys::DeathEffectArmed,
                            Keys::HeardAnywhere, Keys::AtMonster, Keys::Chance,     Keys::Pick,
                            Keys::Sounds,        Keys::OldFrames, Keys::OldSubType, Keys::OldNotSubType};
const KeySet ChanceKeys{Keys::OneIn, Keys::Draw};

constexpr const char* RandFpsDraw = "randFps";
constexpr const char* RandDraw = "rand";
// A sound event's chance: one in 1 to this.
constexpr int MaxOneIn = 100000;
// A pick chooses one of two sounds, or one sound or none.
constexpr size_t PickSounds = 2;
constexpr int MaxSoundChannels = 4;

std::string SetEntry(std::string_view name)
{
    return "sound set " + std::string(name);
}

bool ReadMomentList(const OrderedJson& json, const std::string& key, std::vector<MonsterSoundMoment>& moments,
                    EntryIssues& issues)
{
    if (!json.is_array())
    {
        issues.Fatal(key, "must be a list of moments");
        return false;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::optional<MonsterSoundMoment> moment =
            json[i].is_string() ? FindMonsterSoundMoment(json[i].get<std::string>()) : std::nullopt;
        if (!moment)
        {
            issues.Fatal(key + "[" + std::to_string(i) + "]", "is not a moment (attack, hit, death, stand or walk)");
            return false;
        }
        if (std::find(moments.begin(), moments.end(), *moment) != moments.end())
        {
            issues.Fatal(key, "has " + std::string(GetMonsterSoundMomentName(*moment)) + " more than once");
            return false;
        }
        moments.push_back(*moment);
    }
    std::sort(moments.begin(), moments.end());
    return true;
}

bool ReadCueList(const OrderedJson& json, const std::string& key, std::vector<std::string>& cues, EntryIssues& issues)
{
    if (!json.is_array())
    {
        issues.Fatal(key, "must be a list of cues");
        return false;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        std::string cue;
        if (!Json::ReadText(json[i], key + "[" + std::to_string(i) + "]", cue, issues))
        {
            return false;
        }
        if (std::find(cues.begin(), cues.end(), cue) != cues.end())
        {
            issues.Fatal(key, "has " + cue + " more than once");
            return false;
        }
        cues.push_back(std::move(cue));
    }
    return true;
}

void ReadSounds(const OrderedJson& json, std::vector<MonsterSound>& sounds, EntryIssues& issues)
{
    if (!json.is_array())
    {
        issues.Fatal(Keys::Sounds, "must be a list of sounds");
        return;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::string key = std::string(Keys::Sounds) + "[" + std::to_string(i) + "]";
        const OrderedJson& entry = json[i];
        if (!entry.is_object())
        {
            issues.Fatal(key, "must be an object");
            return;
        }
        MonsterSound sound;
        const auto file = entry.find(Keys::File);
        if (file == entry.end())
        {
            issues.Fatal(key + "." + Keys::File, "missing");
            return;
        }
        if (!Json::ReadPath(*file, key + "." + Keys::File, sound.file, issues))
        {
            return;
        }
        if (const auto id = entry.find(Keys::Id);
            id != entry.end() && !Json::ReadText(*id, key + "." + Keys::Id, sound.id, issues))
        {
            return;
        }
        if (const auto channels = entry.find(Keys::Channels); channels != entry.end())
        {
            const std::optional<int> count = Json::ReadWholeIn(*channels, 1, MaxSoundChannels);
            if (!count)
            {
                issues.Fatal(key + "." + Keys::Channels, Json::RangeText(1, MaxSoundChannels));
                return;
            }
            sound.channels = *count;
        }
        if (const auto spatial = entry.find(Keys::Spatial); spatial != entry.end())
        {
            if (!spatial->is_boolean())
            {
                issues.Fatal(key + "." + Keys::Spatial, "must be true or false");
                return;
            }
            sound.spatial = spatial->get<bool>();
        }
        issues.WarnAboutUnknownKeys(entry, SoundKeys, key + ".");
        sounds.push_back(std::move(sound));
    }
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

bool ReadSoundDraw(const OrderedJson& json, const std::string& key, MonsterSoundDraw& draw, EntryIssues& issues)
{
    const std::optional<MonsterSoundDraw> found = FindSoundDraw(json);
    if (!found)
    {
        issues.Fatal(key, std::string("must be ") + RandFpsDraw + " or " + RandDraw);
        return false;
    }
    draw = *found;
    return true;
}

bool ReadChance(const OrderedJson& json, const std::string& key, MonsterSoundChance& chance, EntryIssues& issues)
{
    if (!json.is_object() || !json.contains(Keys::OneIn) || !json.contains(Keys::Draw))
    {
        issues.Fatal(key, "must be an object with oneIn and draw");
        return false;
    }
    const std::optional<int> oneIn = Json::ReadWholeIn(json[Keys::OneIn], 1, MaxOneIn);
    if (!oneIn)
    {
        issues.Fatal(key + "." + Keys::OneIn, Json::RangeText(1, MaxOneIn));
        return false;
    }
    chance.oneIn = *oneIn;
    if (!ReadSoundDraw(json[Keys::Draw], key + "." + Keys::Draw, chance.draw, issues))
    {
        return false;
    }
    issues.WarnAboutUnknownKeys(json, ChanceKeys, key + ".");
    return true;
}

bool ReadEventSounds(const OrderedJson& json, const std::string& key, std::vector<std::string>& sounds,
                     EntryIssues& issues)
{
    if (!json.is_array() || json.empty())
    {
        issues.Fatal(key, "must be a list of one sound or more");
        return false;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        std::string sound;
        if (!Json::ReadText(json[i], key + "[" + std::to_string(i) + "]", sound, issues))
        {
            return false;
        }
        sounds.push_back(std::move(sound));
    }
    return true;
}

// A true or false field of a sound event; false when it is missing.
bool ReadEventFlag(const OrderedJson& entry, const char* key, const std::string& prefix, bool& value,
                   EntryIssues& issues)
{
    const auto flag = entry.find(key);
    if (flag == entry.end())
    {
        return true;
    }
    if (!flag->is_boolean())
    {
        issues.Fatal(prefix + key, "must be true or false");
        return false;
    }
    value = flag->get<bool>();
    return true;
}

// The fields the model events had before the sound sets: SubType conditions
// became sets of their own (Q-MN78), frames became cues of the models
// (Q-MN79).
bool RefuseOldEventKeys(const OrderedJson& entry, const std::string& prefix, EntryIssues& issues)
{
    bool refused = false;
    for (const char* key : {Keys::OldSubType, Keys::OldNotSubType})
    {
        if (entry.contains(key))
        {
            issues.Fatal(prefix + key, "is gone: a monster names its own sound set (soundSet in Monsters.json)");
            refused = true;
        }
    }
    if (entry.contains(Keys::OldFrames))
    {
        issues.Fatal(prefix + Keys::OldFrames,
                     "is gone: an event plays on cues of the monster's model (cues), which have the frames");
        refused = true;
    }
    return !refused;
}

// When the event plays: in animations, at moments (Q-MN76) or on cues
// (Q-MN79).
bool ReadEventWhen(const OrderedJson& entry, const std::string& prefix, MonsterSoundEvent& event, EntryIssues& issues)
{
    if (entry.contains(Keys::Animations))
    {
        if (!Json::ReadAnimationList(entry[Keys::Animations], prefix + Keys::Animations, event.animations, issues))
        {
            return false;
        }
        if (event.animations.empty())
        {
            issues.Fatal(prefix + Keys::Animations, "must name one animation or more");
            return false;
        }
        return true;
    }
    if (entry.contains(Keys::Cues))
    {
        if (!ReadCueList(entry[Keys::Cues], prefix + Keys::Cues, event.cues, issues))
        {
            return false;
        }
        if (event.cues.empty())
        {
            issues.Fatal(prefix + Keys::Cues, "must name one cue or more");
            return false;
        }
        return true;
    }
    if (!ReadMomentList(entry[Keys::On], prefix + Keys::On, event.moments, issues))
    {
        return false;
    }
    if (event.moments.empty())
    {
        issues.Fatal(prefix + Keys::On, "must name one moment or more");
        return false;
    }
    return true;
}

bool ReadSoundEvent(const OrderedJson& entry, const std::string& key, MonsterSoundEvent& event, EntryIssues& issues)
{
    const int whens = entry.is_object() ? static_cast<int>(entry.contains(Keys::Animations)) +
                                              static_cast<int>(entry.contains(Keys::On)) +
                                              static_cast<int>(entry.contains(Keys::Cues))
                                        : 0;
    if (!entry.is_object() || whens != 1 || !entry.contains(Keys::Sounds))
    {
        issues.Fatal(key, "must be an object with one of animations, on and cues, and sounds");
        return false;
    }
    const std::string prefix = key + ".";
    if (!RefuseOldEventKeys(entry, prefix, issues) || !ReadEventWhen(entry, prefix, event, issues))
    {
        return false;
    }
    if (!ReadEventFlag(entry, Keys::DeathEffectArmed, prefix, event.deathEffectArmed, issues) ||
        !ReadEventFlag(entry, Keys::HeardAnywhere, prefix, event.heardAnywhere, issues) ||
        !ReadEventFlag(entry, Keys::AtMonster, prefix, event.atMonster, issues))
    {
        return false;
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
    if (event.pick && event.sounds.size() > PickSounds)
    {
        issues.Fatal(prefix + Keys::Sounds, "must be one or two sounds with pick");
        return false;
    }
    issues.WarnAboutUnknownKeys(entry, SoundEventKeys, prefix);
    return true;
}

void ReadSoundEvents(const OrderedJson& json, std::vector<MonsterSoundEvent>& events, EntryIssues& issues)
{
    if (!json.is_array())
    {
        issues.Fatal(Keys::SoundEvents, "must be a list of sound events");
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

std::optional<MonsterSoundSet> ReadSet(const OrderedJson& json, size_t index, const std::string& source,
                                       std::vector<ItemDataIssue>& issues)
{
    const std::string entry = std::string(Keys::SoundSets) + "[" + std::to_string(index) + "]";
    const auto name = json.is_object() ? json.find(Keys::Name) : json.end();
    if (!json.is_object() || name == json.end() || !name->is_string() || name->get_ref<const std::string&>().empty())
    {
        issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                          entry + "." + Keys::Name, "missing or not a text"});
        return std::nullopt;
    }
    MonsterSoundSet set;
    set.name = name->get<std::string>();
    EntryIssues setIssues(source, SetEntry(set.name), issues);
    if (const auto sounds = json.find(Keys::Sounds); sounds != json.end())
    {
        ReadSounds(*sounds, set.sounds, setIssues);
    }
    if (const auto events = json.find(Keys::SoundEvents); events != json.end())
    {
        ReadSoundEvents(*events, set.soundEvents, setIssues);
    }
    if (const auto maps = json.find(Keys::Maps); maps != json.end())
    {
        Json::ReadMaps(*maps, Keys::Maps, set.maps, setIssues);
    }
    setIssues.WarnAboutUnknownKeys(json, SetKeys, "");
    if (!setIssues.IsValid())
    {
        return std::nullopt;
    }
    return set;
}

// ---------------------------------------------------------------- writing

OrderedJson WriteMomentList(const std::vector<MonsterSoundMoment>& moments)
{
    OrderedJson json = OrderedJson::array();
    for (const MonsterSoundMoment moment : moments)
    {
        json.push_back(std::string(GetMonsterSoundMomentName(moment)));
    }
    return json;
}

OrderedJson WriteSound(const MonsterSound& sound)
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

OrderedJson WriteSoundDraw(MonsterSoundDraw draw)
{
    return draw == MonsterSoundDraw::Rand ? RandDraw : RandFpsDraw;
}

OrderedJson WriteSoundEvent(const MonsterSoundEvent& event)
{
    OrderedJson json = OrderedJson::object();
    if (!event.animations.empty())
    {
        json[Keys::Animations] = Json::WriteAnimationList(event.animations);
    }
    else if (!event.cues.empty())
    {
        json[Keys::Cues] = event.cues;
    }
    else
    {
        json[Keys::On] = WriteMomentList(event.moments);
    }
    if (event.deathEffectArmed)
    {
        json[Keys::DeathEffectArmed] = true;
    }
    if (event.heardAnywhere)
    {
        json[Keys::HeardAnywhere] = true;
    }
    if (event.atMonster)
    {
        json[Keys::AtMonster] = true;
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

OrderedJson WriteSet(const MonsterSoundSet& set)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Name] = set.name;
    if (!set.sounds.empty())
    {
        OrderedJson sounds = OrderedJson::array();
        for (const MonsterSound& sound : set.sounds)
        {
            sounds.push_back(WriteSound(sound));
        }
        json[Keys::Sounds] = std::move(sounds);
    }
    if (!set.soundEvents.empty())
    {
        OrderedJson events = OrderedJson::array();
        for (const MonsterSoundEvent& event : set.soundEvents)
        {
            events.push_back(WriteSoundEvent(event));
        }
        json[Keys::SoundEvents] = std::move(events);
    }
    if (!set.maps.empty())
    {
        json[Keys::Maps] = Data::Maps::WriteMapRefs(set.maps);
    }
    return json;
}

// Short lists on one line, each sound and event on its own line.
std::string FormatText(std::string text)
{
    using namespace Items::Json;
    text = PutListsOnOneLine(text, Keys::Maps);
    for (const char* key : {Keys::Sounds, Keys::SoundEvents})
    {
        text = PutListEntriesOnOneLine(text, key);
    }
    return text;
}
} // namespace

std::string MonsterSoundSetField(std::string_view name, std::string_view key)
{
    return SetEntry(name) + " " + std::string(key);
}

void ReadMonsterSoundsJson(std::string_view text, const std::string& source, std::vector<MonsterSoundSet>& sets,
                           std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, MonsterSoundsFormatVersion, root, issues))
    {
        return;
    }
    const auto addFileIssue = [&](ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    { issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message}); };
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, Keys::SoundSets}, "",
                                      [&](const std::string& unknown, const std::string& message)
                                      { addFileIssue(ItemDataIssueSeverity::Warning, unknown, message); });
    const auto entries = root.find(Keys::SoundSets);
    if (entries == root.end() || !entries->is_array())
    {
        addFileIssue(ItemDataIssueSeverity::Fatal, Keys::SoundSets, "missing or not a list of sound sets");
        return;
    }
    std::set<std::string, std::less<>> names;
    for (size_t index = 0; index < entries->size(); ++index)
    {
        std::optional<MonsterSoundSet> set = ReadSet((*entries)[index], index, source, issues);
        if (!set)
        {
            continue;
        }
        if (!names.insert(set->name).second)
        {
            addFileIssue(ItemDataIssueSeverity::Fatal, MonsterSoundSetField(set->name, Keys::Name),
                         "is in the list more than once");
            continue;
        }
        sets.push_back(std::move(*set));
    }
}

std::string WriteMonsterSoundsJson(std::span<const MonsterSoundSet> sets)
{
    std::vector<const MonsterSoundSet*> sorted;
    sorted.reserve(sets.size());
    for (const MonsterSoundSet& set : sets)
    {
        sorted.push_back(&set);
    }
    std::stable_sort(sorted.begin(), sorted.end(), [](const MonsterSoundSet* left, const MonsterSoundSet* right)
                     { return left->name < right->name; });
    OrderedJson entries = OrderedJson::array();
    for (const MonsterSoundSet* set : sorted)
    {
        entries.push_back(WriteSet(*set));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = MonsterSoundsFormatVersion;
    root[Keys::SoundSets] = std::move(entries);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    return FormatText(text) + "\n";
}
} // namespace Data::Monsters
