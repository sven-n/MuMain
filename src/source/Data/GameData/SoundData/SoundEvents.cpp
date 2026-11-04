#include "stdafx.h"

#include "SoundEvents.h"

#include "Audio/SoundIds.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <algorithm>
#include <array>
#include <set>
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
constexpr const char* Events = "events";
constexpr const char* Event = "event";
constexpr const char* Rules = "rules";
constexpr const char* Worlds = "worlds";
constexpr const char* Tiles = "tiles";
constexpr const char* SafeZone = "safeZone";
constexpr const char* Holds = "holds";
constexpr const char* Kinds = "kinds";
constexpr const char* Plays = "plays";
} // namespace Keys

const Items::Json::KeySet EventKeys{Keys::Event, Keys::Rules};
const Items::Json::KeySet RuleKeys{Keys::Worlds, Keys::Tiles, Keys::SafeZone, Keys::Holds, Keys::Kinds, Keys::Plays};

constexpr std::array<std::pair<HeldWeapon, std::string_view>, 6> WeaponWords{{
    {HeldWeapon::None, "none"},
    {HeldWeapon::Bow, "bow"},
    {HeldWeapon::Crossbow, "crossbow"},
    {HeldWeapon::MagicCrossbow, "magicCrossbow"},
    {HeldWeapon::SaberOrSpear, "saberOrSpear"},
    {HeldWeapon::Other, "other"},
}};

class EventReader
{
public:
    EventReader(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues) {}

    bool ReadEvent(const OrderedJson& json, const std::string& key, SoundEvent& event)
    {
        if (!json.is_object() || !json.contains(Keys::Event) || !json[Keys::Event].is_string() ||
            json[Keys::Event].get_ref<const std::string&>().empty())
        {
            return Fail(key, "must be an object with the event's name");
        }
        event.event = json[Keys::Event].get<std::string>();
        const auto rules = json.find(Keys::Rules);
        if (rules == json.end() || !rules->is_array())
        {
            return Fail(key + "." + Keys::Rules, "must be a list of rules");
        }
        for (size_t index = 0; index < rules->size(); ++index)
        {
            SoundEventRule rule;
            if (!ReadRule((*rules)[index], key + "." + Keys::Rules + "[" + std::to_string(index) + "]", rule))
            {
                return false;
            }
            event.rules.push_back(std::move(rule));
        }
        Warn(json, EventKeys, key);
        return true;
    }

private:
    bool Fail(const std::string& field, const std::string& message)
    {
        m_issues.push_back(
            {ItemDataIssueSeverity::Fatal, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
        return false;
    }

    void Warn(const OrderedJson& json, const Items::Json::KeySet& keys, const std::string& key)
    {
        Items::Json::WarnAboutUnknownKeys(json, keys, key + ".",
                                          [&](const std::string& field, const std::string& message)
                                          {
                                              m_issues.push_back({ItemDataIssueSeverity::Warning, m_source,
                                                                  ItemDataIssue::NoItem, ItemDataIssue::NoItem, field,
                                                                  message});
                                          });
    }

    bool ReadNumbers(const OrderedJson& json, const std::string& field, long long highest, std::vector<int>& numbers)
    {
        if (!json.is_array())
        {
            return Fail(field, "must be a list of whole numbers");
        }
        for (size_t index = 0; index < json.size(); ++index)
        {
            long long number = 0;
            if (!Items::Json::ReadWholeNumber(json[index], number) || number < 0 || number > highest)
            {
                return Fail(field + "[" + std::to_string(index) + "]",
                            "must be a whole number from 0 to " + std::to_string(highest));
            }
            numbers.push_back(static_cast<int>(number));
        }
        return true;
    }

    bool ReadRule(const OrderedJson& json, const std::string& key, SoundEventRule& rule)
    {
        if (!json.is_object())
        {
            return Fail(key, "must be an object");
        }
        if (const auto worlds = json.find(Keys::Worlds);
            worlds != json.end() && !ReadNumbers(*worlds, key + "." + Keys::Worlds, MaxEventWorld, rule.worlds))
        {
            return false;
        }
        if (const auto tiles = json.find(Keys::Tiles);
            tiles != json.end() && !ReadNumbers(*tiles, key + "." + Keys::Tiles, MaxEventTile, rule.tiles))
        {
            return false;
        }
        if (const auto safeZone = json.find(Keys::SafeZone); safeZone != json.end())
        {
            if (!safeZone->is_boolean())
            {
                return Fail(key + "." + Keys::SafeZone, "must be true or false");
            }
            rule.safeZone = safeZone->get<bool>();
        }
        if (const auto holds = json.find(Keys::Holds); holds != json.end())
        {
            rule.holds = holds->is_string() ? FindHeldWeapon(holds->get_ref<const std::string&>()) : std::nullopt;
            if (!rule.holds)
            {
                return Fail(key + "." + Keys::Holds,
                            "must be none, bow, crossbow, magicCrossbow, saberOrSpear or other");
            }
        }
        if (const auto kinds = json.find(Keys::Kinds); kinds != json.end())
        {
            if (!kinds->is_array())
            {
                return Fail(key + "." + Keys::Kinds, "must be a list of click kinds");
            }
            for (size_t index = 0; index < kinds->size(); ++index)
            {
                const OrderedJson& word = (*kinds)[index];
                if (!word.is_string() || !IsClickKindWord(word.get_ref<const std::string&>()) ||
                    std::find(rule.kinds.begin(), rule.kinds.end(), word.get_ref<const std::string&>()) !=
                        rule.kinds.end())
                {
                    return Fail(key + "." + Keys::Kinds + "[" + std::to_string(index) + "]",
                                "must be a click kind, once (lower-case letters, digits and -)");
                }
                rule.kinds.push_back(word.get<std::string>());
            }
        }
        const auto plays = json.find(Keys::Plays);
        if (plays == json.end() || !plays->is_array() || plays->empty())
        {
            return Fail(key + "." + Keys::Plays, "must be a list of one SOUND_ id or more");
        }
        for (size_t index = 0; index < plays->size(); ++index)
        {
            const std::string field = key + "." + Keys::Plays + "[" + std::to_string(index) + "]";
            const OrderedJson& id = (*plays)[index];
            const std::optional<ESound> number =
                id.is_string() ? ::Audio::SoundIds::Find(id.get_ref<const std::string&>()) : std::nullopt;
            if (!number)
            {
                return Fail(field, "must be a SOUND_ id of the game");
            }
            rule.plays.push_back(id.get<std::string>());
            rule.numbers.push_back(static_cast<int>(*number));
        }
        Warn(json, RuleKeys, key);
        return true;
    }

    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

bool Allows(const std::vector<int>& list, int value)
{
    return list.empty() || std::find(list.begin(), list.end(), value) != list.end();
}

OrderedJson WriteRule(const SoundEventRule& rule)
{
    OrderedJson json = OrderedJson::object();
    if (!rule.worlds.empty())
    {
        json[Keys::Worlds] = rule.worlds;
    }
    if (!rule.tiles.empty())
    {
        json[Keys::Tiles] = rule.tiles;
    }
    if (rule.safeZone)
    {
        json[Keys::SafeZone] = *rule.safeZone;
    }
    if (rule.holds)
    {
        json[Keys::Holds] = std::string(HeldWeaponWord(*rule.holds));
    }
    if (!rule.kinds.empty())
    {
        json[Keys::Kinds] = rule.kinds;
    }
    json[Keys::Plays] = rule.plays;
    return json;
}
} // namespace

void ReadSoundEventsJson(std::string_view text, const std::string& source, std::vector<SoundEvent>& events,
                         std::vector<Items::ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, SoundEventsFormatVersion, root, issues))
    {
        return;
    }
    const auto list = root.find(Keys::Events);
    if (list == root.end() || !list->is_array())
    {
        issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                          Keys::Events, "missing or not a list of events"});
        return;
    }
    EventReader reader(source, issues);
    std::set<std::string> names;
    for (size_t index = 0; index < list->size(); ++index)
    {
        const std::string key = std::string(Keys::Events) + "[" + std::to_string(index) + "]";
        SoundEvent event;
        if (!reader.ReadEvent((*list)[index], key, event))
        {
            continue;
        }
        if (!names.insert(event.event).second)
        {
            issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                              key + "." + Keys::Event, event.event + " is named by another event too"});
            continue;
        }
        events.push_back(std::move(event));
    }
}

std::string WriteSoundEventsJson(std::span<const SoundEvent> events)
{
    OrderedJson list = OrderedJson::array();
    for (const SoundEvent& event : events)
    {
        OrderedJson rules = OrderedJson::array();
        for (const SoundEventRule& rule : event.rules)
        {
            rules.push_back(WriteRule(rule));
        }
        OrderedJson json = OrderedJson::object();
        json[Keys::Event] = event.event;
        json[Keys::Rules] = std::move(rules);
        list.push_back(std::move(json));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = SoundEventsFormatVersion;
    root[Keys::Events] = std::move(list);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    return Items::Json::PutListEntriesOnOneLine(text, Keys::Rules) + "\n";
}

const SoundEventRule* MatchRule(const SoundEvent& event, const SoundEventContext& context)
{
    for (const SoundEventRule& rule : event.rules)
    {
        if (Allows(rule.worlds, context.world) && Allows(rule.tiles, context.tile) &&
            (!rule.safeZone || *rule.safeZone == context.safeZone) && (!rule.holds || *rule.holds == context.holds) &&
            (rule.kinds.empty() || std::find(rule.kinds.begin(), rule.kinds.end(), context.kind) != rule.kinds.end()))
        {
            return &rule;
        }
    }
    return nullptr;
}

bool IsClickKindWord(std::string_view word)
{
    return !word.empty() && std::all_of(word.begin(), word.end(), [](char c)
                                        { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-'; });
}

std::string_view HeldWeaponWord(HeldWeapon weapon)
{
    for (const auto& [kind, word] : WeaponWords)
    {
        if (kind == weapon)
        {
            return word;
        }
    }
    return {};
}

std::optional<HeldWeapon> FindHeldWeapon(std::string_view word)
{
    for (const auto& [kind, weaponWord] : WeaponWords)
    {
        if (weaponWord == word)
        {
            return kind;
        }
    }
    return std::nullopt;
}
} // namespace Data::Sounds
