#include "stdafx.h"

#include "BuffsJson.h"

#include "BuffAttributes.h"

#include "Core/Globals/_define.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemOptionsJson.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <utility>

namespace Data::Buffs
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* Buffs = "buffs";
constexpr const char* Number = "number";
constexpr const char* Name = "name";
constexpr const char* Kind = "kind";
constexpr const char* Group = "group";
constexpr const char* DisplayName = "displayName";
constexpr const char* Description = "description";
constexpr const char* Item = "item";
constexpr const char* Duration = "duration";
constexpr const char* PowerUps = "powerUps";
constexpr const char* Attribute = "attribute";
constexpr const char* ValuesFrom = "valuesFrom";
constexpr const char* MultipliedBy = "multipliedBy";
constexpr const char* ValueOf = "valueOf";
constexpr const char* Values = "values";
constexpr const char* Per = "per";
constexpr const char* Times = "times";
constexpr const char* Divisor = "divisor";
constexpr const char* Maximum = "maximum";
constexpr const char* Family = "family";
constexpr const char* Tier = "tier";
constexpr const char* Hidden = "hidden";
constexpr const char* CancelSkill = "cancelSkill";
} // namespace Keys

constexpr const char* BuffKindName = "buff";
constexpr const char* DebuffKindName = "debuff";
constexpr const char* OwnerName = "owner";
constexpr const char* CasterName = "caster";
// The longest duration a buff can have, a year: the vitality items last 30 days.
constexpr int MaxDurationSeconds = 365 * 24 * 60 * 60;
// A buff's group is a byte, as in the old BuffEffect files.
constexpr int MaxBuffGroup = 255;

const Items::Json::KeySet EntryKeys{Keys::Number,      Keys::Name,        Keys::Kind,   Keys::Group,
                                    Keys::DisplayName, Keys::Description, Keys::Item,   Keys::ValuesFrom,
                                    Keys::Duration,    Keys::PowerUps,    Keys::Family, Keys::Tier,
                                    Keys::Hidden,      Keys::CancelSkill};
const Items::Json::KeySet ItemKeys{Keys::Group, Keys::Number};
const Items::Json::KeySet PowerUpKeys{Keys::Attribute,
                                      Items::ItemOptionAddKeys[0],
                                      Items::ItemOptionAddKeys[1],
                                      Items::ItemOptionAddKeys[2],
                                      Keys::MultipliedBy,
                                      Keys::ValueOf};
const Items::Json::KeySet ValueKeys{Keys::Values, Keys::Per, Keys::Maximum};
const Items::Json::KeySet TermKeys{Keys::Attribute, Keys::Times, Keys::Divisor};
// An owner's buff keeps Webzen's whole numbers, which the item tooltips show as they are.
constexpr double MaxOwnerValue = 65535;
// A caster's value and maximum stay within this either way (OpenMU's largest constants are Berserker's 140 and 160
// damage), so a computed total stays within an int.
constexpr double MaxCasterValue = 1000000;
constexpr double Huge = std::numeric_limits<double>::max();

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& source,
              const std::string& field, const std::string& message)
{
    issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
}

// The fields of one entry; false, with a fatal problem added, at the first value of the wrong form.
class EntryReader
{
public:
    EntryReader(size_t index, const std::string& source, std::vector<ItemDataIssue>& issues)
        : m_index(index), m_source(source), m_issues(issues)
    {
    }

    bool Read(const OrderedJson& json, BuffDefinition& buff)
    {
        if (!json.is_object())
        {
            return Fail("", "must be an object");
        }
        return ReadWhole(json, Keys::Number, 1, MaxBuffNumber, buff.number) && ReadName(json, buff.name) &&
               ReadKind(json, buff.kind) && ReadWhole(json, Keys::Group, 0, MaxBuffGroup, buff.group) &&
               ReadText(json, Keys::DisplayName, true, buff.displayName) &&
               ReadText(json, Keys::Description, false, buff.description) && ReadItem(json, buff.item) &&
               ReadValuesFrom(json, buff.valuesFrom) && ReadDuration(json, buff.valuesFrom, buff.duration) &&
               ReadPowerUps(json, buff.valuesFrom, buff.powerUps) && ReadFamily(json, buff) &&
               ReadHidden(json, buff.hidden) && ReadCancelSkill(json, buff.cancelSkill) &&
               WarnUnknown(json, EntryKeys, "");
    }

private:
    bool Fail(const std::string& key, const std::string& message)
    {
        AddFatal(BuffField(m_index, key), message);
        return false;
    }

    void AddFatal(const std::string& field, const std::string& message)
    {
        AddIssue(m_issues, ItemDataIssueSeverity::Fatal, m_source, field, message);
    }

    // The whole number at `key` of `json`, which is the entry's field `prefix` ("" for the entry itself).
    bool ReadWhole(const OrderedJson& json, const std::string& prefix, const char* key, int low, int high, int& value)
    {
        const std::string field = BuffField(m_index, prefix.empty() ? std::string(key) : prefix + "." + key);
        // A missing field reads as null, which is no whole number.
        static const OrderedJson Missing;
        const auto found = json.find(key);
        return Items::Json::ReadWholeNumberInRange(found == json.end() ? Missing : *found, field, low, high, value,
                                                   [this](const std::string& name, const std::string& message)
                                                   { AddFatal(name, message); });
    }

    bool ReadWhole(const OrderedJson& json, const char* key, int low, int high, int& value)
    {
        return ReadWhole(json, "", key, low, high, value);
    }

    bool ReadName(const OrderedJson& json, std::string& name)
    {
        return Items::Json::ReadNameField(json, Keys::Name, BuffField(m_index, ""), name,
                                          [this](const std::string& field, const std::string& message)
                                          { AddFatal(field, message); });
    }

    bool ReadKind(const OrderedJson& json, BuffKind& kind)
    {
        const auto field = json.find(Keys::Kind);
        const std::string text = field != json.end() && field->is_string() ? field->get<std::string>() : "";
        if (text == BuffKindName)
        {
            kind = BuffKind::Buff;
            return true;
        }
        if (text == DebuffKindName)
        {
            kind = BuffKind::Debuff;
            return true;
        }
        return Fail(Keys::Kind, std::string("must be ") + BuffKindName + " or " + DebuffKindName);
    }

    bool ReadText(const OrderedJson& json, const char* key, bool required, LocalizedString& text)
    {
        const auto field = json.find(key);
        if (field == json.end())
        {
            return !required || Fail(key, "missing");
        }
        bool valid = Items::Json::ReadLocalizedString(
            *field, BuffField(m_index, key), text, [this](const std::string& name, const std::string& message)
            { AddIssue(m_issues, ItemDataIssueSeverity::Fatal, m_source, name, message); });
        if (valid && text.GetNeutral().empty())
        {
            valid = Fail(key, "must have an English text");
        }
        return valid;
    }

    bool ReadItem(const OrderedJson& json, std::optional<BuffItemLink>& item)
    {
        const auto field = json.find(Keys::Item);
        if (field == json.end())
        {
            return true;
        }
        if (!field->is_object())
        {
            return Fail(Keys::Item, "must be an object with group and number");
        }
        BuffItemLink link;
        if (!ReadWhole(*field, Keys::Item, Keys::Group, 0, MAX_ITEM_TYPE - 1, link.group) ||
            !ReadWhole(*field, Keys::Item, Keys::Number, 0, MAX_ITEM_INDEX - 1, link.number))
        {
            return false;
        }
        item = link;
        return WarnUnknown(*field, ItemKeys, Keys::Item);
    }

    bool ReadValuesFrom(const OrderedJson& json, BuffValuesFrom& valuesFrom)
    {
        const auto field = json.find(Keys::ValuesFrom);
        if (field == json.end())
        {
            return true;
        }
        const std::string text = field->is_string() ? field->get<std::string>() : "";
        if (text == OwnerName || text == CasterName)
        {
            valuesFrom = text == CasterName ? BuffValuesFrom::Caster : BuffValuesFrom::Owner;
            return true;
        }
        return Fail(Keys::ValuesFrom, std::string("must be ") + OwnerName + " or " + CasterName);
    }

    // The decimal number at `key` of `json`, which is the field `field`; false, with a fatal problem added, when it is
    // no number or out of `low` to `high`.
    bool ReadNumber(const OrderedJson& json, const std::string& field, const char* key, double low, double high,
                    double& number)
    {
        const auto found = json.find(key);
        const std::string name = field + "." + key;
        if (found == json.end() || !found->is_number())
        {
            AddFatal(name, "must be a number");
            return false;
        }
        number = found->get<double>();
        if (number < low || number > high)
        {
            const bool bounded = low > -Huge && high < Huge;
            AddFatal(name, bounded ? "must be from " + std::to_string(static_cast<long long>(low)) + " to " +
                                         std::to_string(static_cast<long long>(high))
                                   : "is out of range");
            return false;
        }
        return true;
    }

    bool RequireWhole(const std::string& field, double number)
    {
        if (number == std::floor(number))
        {
            return true;
        }
        AddFatal(field, "must be a whole number: an owner's buff keeps Webzen's whole numbers");
        return false;
    }

    // A value of the buffs' format (BuffValue) at `field`; false when it has a fatal problem. An owner's buff keeps
    // whole numbers from 0 to `limit` (Webzen's data, which the item tooltips show); a caster's takes OpenMU's
    // decimals and negatives (buffs design B17) from -`limit` to `limit`, its maximum too.
    bool ReadValue(const OrderedJson& json, const std::string& field, BuffValuesFrom valuesFrom, BuffValue& value,
                   std::optional<double> limit = std::nullopt)
    {
        if (!json.is_object())
        {
            AddFatal(field, R"(must be an object: {"values": 60})");
            return false;
        }
        if (const auto values = json.find(Keys::Values); values != json.end() && values->is_array())
        {
            // A list holds one value per item level from level 1, and a buff has no item level.
            AddFatal(field, "its values must be one number, not a list: a buff has no item level");
            return false;
        }
        const bool owner = valuesFrom == BuffValuesFrom::Owner;
        const double high = limit.value_or(owner ? MaxOwnerValue : MaxCasterValue);
        if (!ReadNumber(json, field, Keys::Values, owner ? 0 : -high, high, value.values) ||
            (owner && !RequireWhole(field + "." + Keys::Values, value.values)))
        {
            return false;
        }
        if (json.contains(Keys::Maximum))
        {
            double maximum = 0;
            if (!ReadNumber(json, field, Keys::Maximum, -high, high, maximum))
            {
                return false;
            }
            value.maximum = maximum;
        }
        if (const auto per = json.find(Keys::Per); per != json.end())
        {
            if (!per->is_array())
            {
                AddFatal(field + "." + Keys::Per, R"(must be a list: [{"attribute": "totalEnergy", "divisor": 7}])");
                return false;
            }
            for (size_t index = 0; index < per->size(); ++index)
            {
                BuffTerm term;
                if (!ReadTerm((*per)[index], field + "." + Keys::Per + "[" + std::to_string(index) + "]", owner, term))
                {
                    return false;
                }
                value.per.push_back(std::move(term));
            }
        }
        WarnUnknownAt(json, ValueKeys, field);
        return true;
    }

    bool ReadTerm(const OrderedJson& json, const std::string& field, bool owner, BuffTerm& term)
    {
        if (!json.is_object())
        {
            AddFatal(field, R"(must be an object: {"attribute": "totalEnergy", "divisor": 7})");
            return false;
        }
        if (!Items::Json::ReadNameField(json, Keys::Attribute, field, term.attribute,
                                        [this](const std::string& name, const std::string& message)
                                        { AddFatal(name, message); }))
        {
            return false;
        }
        const bool hasTimes = json.contains(Keys::Times);
        if (hasTimes == json.contains(Keys::Divisor))
        {
            AddFatal(field, "needs exactly one of times and divisor");
            return false;
        }
        double& number = hasTimes ? term.times : term.divisor;
        const char* key = hasTimes ? Keys::Times : Keys::Divisor;
        if (!ReadNumber(json, field, key, std::numeric_limits<double>::min(), Huge, number) ||
            (owner && !RequireWhole(field + "." + key, number)))
        {
            return false;
        }
        WarnUnknownAt(json, TermKeys, field);
        return true;
    }

    bool ReadDuration(const OrderedJson& json, BuffValuesFrom valuesFrom, std::optional<BuffValue>& duration)
    {
        const auto field = json.find(Keys::Duration);
        if (field == json.end())
        {
            return true;
        }
        BuffValue value;
        if (!ReadValue(*field, BuffField(m_index, Keys::Duration), valuesFrom, value, MaxDurationSeconds))
        {
            return false;
        }
        if (valuesFrom == BuffValuesFrom::Owner && !value.per.empty())
        {
            // An owner's buff (an item) lasts its fixed time; only a caster's grows with the caster's attributes.
            return Fail(Keys::Duration, "must be a number of seconds without per terms");
        }
        if (value.values < 0)
        {
            return Fail(Keys::Duration, "must not be below 0 seconds");
        }
        duration = std::move(value);
        return true;
    }

    bool ReadPowerUps(const OrderedJson& json, BuffValuesFrom valuesFrom, std::vector<BuffPowerUp>& powerUps)
    {
        const auto field = json.find(Keys::PowerUps);
        if (field == json.end())
        {
            return true;
        }
        if (!field->is_array() || field->empty())
        {
            return Fail(Keys::PowerUps, R"(must be a list: [{"attribute": "defenseBase", "add": {"values": 60}}])");
        }
        for (size_t index = 0; index < field->size(); ++index)
        {
            const std::string key = std::string(Keys::PowerUps) + "[" + std::to_string(index) + "]";
            const OrderedJson& entry = (*field)[index];
            if (!entry.is_object())
            {
                return Fail(key, R"(must be an object: {"attribute": "defenseBase", "add": {"values": 60}})");
            }
            BuffPowerUp powerUp;
            if (!Items::Json::ReadNameField(entry, Keys::Attribute, BuffField(m_index, key), powerUp.attribute,
                                            [this](const std::string& errorField, const std::string& message)
                                            { AddFatal(errorField, message); }))
            {
                return false;
            }
            const OrderedJson* value = nullptr;
            for (size_t way = 0; way < Items::ItemOptionAddKeys.size(); ++way)
            {
                if (const auto found = entry.find(Items::ItemOptionAddKeys[way]); found != entry.end())
                {
                    if (value != nullptr)
                    {
                        return Fail(key, "needs exactly one of add, addFinal and percent");
                    }
                    value = &*found;
                    powerUp.add = static_cast<Items::ItemOptionAdd>(way);
                }
            }
            if (value == nullptr)
            {
                return Fail(key, "needs exactly one of add, addFinal and percent");
            }
            if (!ReadValue(*value,
                           BuffField(m_index, key) + "." + Items::ItemOptionAddKeys[static_cast<size_t>(powerUp.add)],
                           valuesFrom, powerUp.value) ||
                !ReadChain(entry, key, Keys::MultipliedBy, powerUp.multipliedBy) ||
                !ReadChainList(entry, key, Keys::ValueOf, powerUp.valueOf))
            {
                return false;
            }
            WarnUnknown(entry, PowerUpKeys, key);
            powerUps.push_back(std::move(powerUp));
        }
        return true;
    }

    // The attribute of another power-up at `key` of the power-up `entry` (the entry's field `powerUpKey`), if it has
    // one.
    bool ReadChain(const OrderedJson& entry, const std::string& powerUpKey, const char* key, std::string& attribute)
    {
        return !entry.contains(key) ||
               Items::Json::ReadNameField(entry, key, BuffField(m_index, powerUpKey), attribute,
                                          [this](const std::string& field, const std::string& message)
                                          { AddFatal(field, message); });
    }

    // The attributes of other power-ups at `key`: one name, or a list of them.
    bool ReadChainList(const OrderedJson& entry, const std::string& powerUpKey, const char* key,
                       std::vector<std::string>& attributes)
    {
        const auto found = entry.find(key);
        if (found == entry.end())
        {
            return true;
        }
        const auto isName = [](const OrderedJson& name)
        { return name.is_string() && Items::Json::IsName(name.get_ref<const std::string&>()); };
        if (isName(*found))
        {
            attributes.push_back(found->get<std::string>());
            return true;
        }
        if (!found->is_array() || found->empty() || !std::all_of(found->begin(), found->end(), isName))
        {
            AddFatal(BuffField(m_index, powerUpKey) + "." + key,
                     R"(must be a power-up's attribute or a list of them: ["berserkerManaMultiplier"])");
            return false;
        }
        for (const OrderedJson& name : *found)
        {
            attributes.push_back(name.get<std::string>());
        }
        return true;
    }

    bool ReadFamily(const OrderedJson& json, BuffDefinition& buff)
    {
        const auto family = json.find(Keys::Family);
        const auto tier = json.find(Keys::Tier);
        if (family == json.end())
        {
            return tier == json.end() || Fail(Keys::Tier, "needs a family");
        }
        if (!family->is_string() || !Items::Json::IsName(family->get_ref<const std::string&>()))
        {
            return Fail(Keys::Family, "must be a name of letters and digits");
        }
        buff.family = family->get<std::string>();
        return tier == json.end() || ReadWhole(json, Keys::Tier, 0, MaxBuffTier, buff.tier);
    }

    bool ReadHidden(const OrderedJson& json, bool& hidden)
    {
        const auto field = json.find(Keys::Hidden);
        if (field == json.end())
        {
            return true;
        }
        if (!field->is_boolean())
        {
            return Fail(Keys::Hidden, "must be true or false");
        }
        hidden = field->get<bool>();
        return true;
    }

    bool ReadCancelSkill(const OrderedJson& json, int& skill)
    {
        return !json.contains(Keys::CancelSkill) || ReadWhole(json, Keys::CancelSkill, 1, MAX_SKILLS - 1, skill);
    }

    // Warns about the keys of `json`, the field `field` (a full field name), that `keys` lacks.
    void WarnUnknownAt(const OrderedJson& json, const Items::Json::KeySet& keys, const std::string& field)
    {
        Items::Json::WarnAboutUnknownKeys(
            json, keys, field + ".", [&](const std::string& name, const std::string& message)
            { AddIssue(m_issues, ItemDataIssueSeverity::Warning, m_source, name, message); });
    }

    bool WarnUnknown(const OrderedJson& json, const Items::Json::KeySet& keys, const std::string& key)
    {
        Items::Json::WarnAboutUnknownKeys(
            json, keys, BuffField(m_index, key) + ".", [&](const std::string& field, const std::string& message)
            { AddIssue(m_issues, ItemDataIssueSeverity::Warning, m_source, field, message); });
        return true;
    }

    size_t m_index;
    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

// A number as the file writes it: a whole one without a decimal point.
OrderedJson WriteNumber(double number)
{
    if (number == std::floor(number) && std::abs(number) < 1e15)
    {
        return static_cast<long long>(number);
    }
    return number;
}

OrderedJson WriteValue(const BuffValue& value)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Values] = WriteNumber(value.values);
    if (!value.per.empty())
    {
        OrderedJson per = OrderedJson::array();
        for (const BuffTerm& term : value.per)
        {
            OrderedJson entry = OrderedJson::object();
            entry[Keys::Attribute] = term.attribute;
            if (term.times > 0)
            {
                entry[Keys::Times] = WriteNumber(term.times);
            }
            else
            {
                entry[Keys::Divisor] = WriteNumber(term.divisor);
            }
            per.push_back(std::move(entry));
        }
        json[Keys::Per] = std::move(per);
    }
    if (value.maximum)
    {
        json[Keys::Maximum] = WriteNumber(*value.maximum);
    }
    return json;
}

OrderedJson WriteEntry(const BuffDefinition& buff)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Number] = buff.number;
    json[Keys::Name] = buff.name;
    json[Keys::Kind] = buff.kind == BuffKind::Debuff ? DebuffKindName : BuffKindName;
    json[Keys::Group] = buff.group;
    json[Keys::DisplayName] = Items::Json::WriteLocalizedString(buff.displayName);
    if (!buff.description.IsEmpty())
    {
        json[Keys::Description] = Items::Json::WriteLocalizedString(buff.description);
    }
    if (buff.item)
    {
        OrderedJson item = OrderedJson::object();
        item[Keys::Group] = buff.item->group;
        item[Keys::Number] = buff.item->number;
        json[Keys::Item] = std::move(item);
    }
    if (buff.valuesFrom == BuffValuesFrom::Caster)
    {
        json[Keys::ValuesFrom] = CasterName;
    }
    if (buff.duration)
    {
        json[Keys::Duration] = WriteValue(*buff.duration);
    }
    if (!buff.powerUps.empty())
    {
        OrderedJson powerUps = OrderedJson::array();
        for (const BuffPowerUp& powerUp : buff.powerUps)
        {
            OrderedJson entry = OrderedJson::object();
            entry[Keys::Attribute] = powerUp.attribute;
            entry[Items::ItemOptionAddKeys[static_cast<size_t>(powerUp.add)]] = WriteValue(powerUp.value);
            if (!powerUp.multipliedBy.empty())
            {
                entry[Keys::MultipliedBy] = powerUp.multipliedBy;
            }
            if (powerUp.valueOf.size() == 1)
            {
                entry[Keys::ValueOf] = powerUp.valueOf.front();
            }
            else if (!powerUp.valueOf.empty())
            {
                entry[Keys::ValueOf] = powerUp.valueOf;
            }
            powerUps.push_back(std::move(entry));
        }
        json[Keys::PowerUps] = std::move(powerUps);
    }
    if (!buff.family.empty())
    {
        json[Keys::Family] = buff.family;
        if (buff.tier != 0)
        {
            json[Keys::Tier] = buff.tier;
        }
    }
    if (buff.hidden)
    {
        json[Keys::Hidden] = true;
    }
    if (buff.cancelSkill != 0)
    {
        json[Keys::CancelSkill] = buff.cancelSkill;
    }
    return json;
}
} // namespace

namespace
{
void WarnAboutUnreadTerms(const BuffValue& value, const std::string& field, const std::string& source,
                          std::vector<ItemDataIssue>& issues)
{
    for (size_t term = 0; term < value.per.size(); ++term)
    {
        if (FindBuffTermAttribute(value.per[term].attribute) == BuffTermAttribute::Unread)
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, source,
                     field + "." + Keys::Per + "[" + std::to_string(term) + "]." + Keys::Attribute,
                     "the client reads no hero attribute " + value.per[term].attribute + ": the term adds nothing");
        }
    }
}

// The index of the buff's power-up of that attribute other than `self`, or NoPowerUp: the one the buff table chains
// to (BuffDatabase::Build).
int FindOtherPowerUp(const BuffDefinition& buff, std::string_view attribute, size_t self)
{
    for (size_t index = 0; index < buff.powerUps.size(); ++index)
    {
        if (index != self && buff.powerUps[index].attribute == attribute)
        {
            return static_cast<int>(index);
        }
    }
    return NoPowerUp;
}

// multipliedBy and valueOf name another power-up of the buff, and no chain leads back to where it starts: the values
// are computed along them (buffs design B21).
void ValidateChains(const BuffDefinition& buff, size_t entry, const std::string& source,
                    std::vector<ItemDataIssue>& issues)
{
    for (size_t index = 0; index < buff.powerUps.size(); ++index)
    {
        const BuffPowerUp& powerUp = buff.powerUps[index];
        const std::string field = BuffField(entry, std::string(Keys::PowerUps) + "[" + std::to_string(index) + "]");
        std::vector<std::pair<std::string, const std::string*>> names;
        if (!powerUp.multipliedBy.empty())
        {
            names.emplace_back(Keys::MultipliedBy, &powerUp.multipliedBy);
        }
        for (size_t at = 0; at < powerUp.valueOf.size(); ++at)
        {
            names.emplace_back(powerUp.valueOf.size() == 1
                                   ? std::string(Keys::ValueOf)
                                   : std::string(Keys::ValueOf) + "[" + std::to_string(at) + "]",
                               &powerUp.valueOf[at]);
        }
        if (powerUp.valueOf.size() > MaxValueOf)
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, source, field + "." + Keys::ValueOf,
                     "names " + std::to_string(powerUp.valueOf.size()) + " power-ups: the values add only the first " +
                         std::to_string(MaxValueOf));
        }
        for (size_t at = 1; at < powerUp.valueOf.size(); ++at)
        {
            if (std::find(powerUp.valueOf.begin(), powerUp.valueOf.begin() + static_cast<std::ptrdiff_t>(at),
                          powerUp.valueOf[at]) != powerUp.valueOf.begin() + static_cast<std::ptrdiff_t>(at))
            {
                AddIssue(issues, ItemDataIssueSeverity::Error, source,
                         field + "." + Keys::ValueOf + "[" + std::to_string(at) + "]",
                         powerUp.valueOf[at] + " is named twice: its value would be added twice");
            }
        }
        for (const auto& [key, name] : names)
        {
            if (FindOtherPowerUp(buff, *name, index) == NoPowerUp)
            {
                AddIssue(issues, ItemDataIssueSeverity::Fatal, source, field + "." + key,
                         *name + " is no other power-up of this buff");
            }
        }
        // Walk the chains from this power-up, each power-up once: it is on a loop when they reach it again.
        std::vector<bool> seen(buff.powerUps.size(), false);
        std::vector<size_t> next{index};
        bool loops = false;
        while (!next.empty() && !loops)
        {
            const BuffPowerUp& step = buff.powerUps[next.back()];
            const size_t at = next.back();
            next.pop_back();
            std::vector<const std::string*> chained{&step.multipliedBy};
            for (const std::string& name : step.valueOf)
            {
                chained.push_back(&name);
            }
            for (const std::string* name : chained)
            {
                const int other = name->empty() ? NoPowerUp : FindOtherPowerUp(buff, *name, at);
                if (other == static_cast<int>(index))
                {
                    loops = true;
                }
                else if (other != NoPowerUp && !seen[static_cast<size_t>(other)])
                {
                    seen[static_cast<size_t>(other)] = true;
                    next.push_back(static_cast<size_t>(other));
                }
            }
        }
        if (loops)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, field,
                     "its multipliedBy and valueOf lead back to it: which value comes first?");
        }
    }
}
} // namespace

std::string BuffField(size_t entry, std::string_view key)
{
    std::string field = std::string(Keys::Buffs) + "[" + std::to_string(entry) + "]";
    return key.empty() ? field : field + "." + std::string(key);
}

void ReadBuffsJson(std::string_view text, const std::string& source, std::vector<BuffDefinition>& buffs,
                   std::vector<Items::ItemDataIssue>& issues, std::vector<size_t>* fileEntries)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, BuffsFormatVersion, root, issues))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, Keys::Buffs}, "",
                                      [&](const std::string& field, const std::string& message)
                                      { AddIssue(issues, ItemDataIssueSeverity::Warning, source, field, message); });
    const auto entries = root.find(Keys::Buffs);
    if (entries == root.end() || !entries->is_array())
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, source, Keys::Buffs, "missing or not a list of buffs");
        return;
    }
    for (size_t index = 0; index < entries->size(); ++index)
    {
        BuffDefinition buff;
        if (EntryReader(index, source, issues).Read((*entries)[index], buff))
        {
            buffs.push_back(std::move(buff));
            if (fileEntries != nullptr)
            {
                fileEntries->push_back(index);
            }
        }
    }
}

std::string WriteBuffsJson(std::span<const BuffDefinition> buffs)
{
    OrderedJson entries = OrderedJson::array();
    for (const BuffDefinition& buff : buffs)
    {
        entries.push_back(WriteEntry(buff));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = BuffsFormatVersion;
    root[Keys::Buffs] = std::move(entries);
    return root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace) + "\n";
}

void ValidateBuffs(std::span<const BuffDefinition> buffs, std::span<const size_t> entries, const std::string& source,
                   std::vector<Items::ItemDataIssue>& issues)
{
    const auto fileEntry = [&](size_t position) { return position < entries.size() ? entries[position] : position; };
    std::map<int, size_t> numbers;
    std::map<std::string, size_t, std::less<>> names;
    std::set<std::pair<std::string, int>> tiers;
    std::map<std::string, BuffKind, std::less<>> familyKinds;
    for (size_t position = 0; position < buffs.size(); ++position)
    {
        const BuffDefinition& buff = buffs[position];
        const size_t entry = fileEntry(position);
        if (const auto [found, added] = numbers.emplace(buff.number, position); !added)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, BuffField(entry, Keys::Number),
                     std::to_string(buff.number) + " is buffs[" + std::to_string(fileEntry(found->second)) + "] too");
        }
        if (const auto [found, added] = names.emplace(buff.name, position); !added)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, BuffField(entry, Keys::Name),
                     buff.name + " is buffs[" + std::to_string(fileEntry(found->second)) + "] too");
        }
        if (!buff.family.empty() && !tiers.emplace(buff.family, buff.tier).second)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, BuffField(entry, Keys::Tier),
                     "family " + buff.family + " has tier " + std::to_string(buff.tier) + " twice: which would show?");
        }
        if (!buff.family.empty() && familyKinds.emplace(buff.family, buff.kind).first->second != buff.kind)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, BuffField(entry, Keys::Kind),
                     "family " + buff.family + " has buffs and debuffs: its tiers are one buff");
        }
        if (buff.kind == BuffKind::Debuff && !buff.powerUps.empty())
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, source, BuffField(entry, Keys::PowerUps),
                     "the character stats don't count a debuff's power-ups: a value raises its attribute, and the "
                     "format has no decrease yet");
        }
        for (size_t index = 0; index < buff.powerUps.size(); ++index)
        {
            if (!IsBuffAttribute(buff.powerUps[index].attribute))
            {
                AddIssue(issues, ItemDataIssueSeverity::Error, source,
                         BuffField(entry,
                                   std::string(Keys::PowerUps) + "[" + std::to_string(index) + "]." + Keys::Attribute),
                         buff.powerUps[index].attribute + " is no attribute a buff can raise (BuffAttributes.h)");
            }
            if (buff.powerUps[index].add != Items::ItemOptionAdd::Add)
            {
                AddIssue(issues, ItemDataIssueSeverity::Warning, source,
                         BuffField(entry, std::string(Keys::PowerUps) + "[" + std::to_string(index) + "]"),
                         std::string(Items::ItemOptionAddKeys[static_cast<size_t>(buff.powerUps[index].add)]) +
                             " is read but the client adds every buff value as add: the value counts as a flat bonus");
            }
            WarnAboutUnreadTerms(
                buff.powerUps[index].value,
                BuffField(entry, std::string(Keys::PowerUps) + "[" + std::to_string(index) + "]." +
                                     Items::ItemOptionAddKeys[static_cast<size_t>(buff.powerUps[index].add)]),
                source, issues);
        }
        if (buff.duration)
        {
            WarnAboutUnreadTerms(*buff.duration, BuffField(entry, Keys::Duration), source, issues);
        }
        ValidateChains(buff, entry, source, issues);
    }
}

void ValidateBuffItems(std::span<const BuffDefinition> buffs, std::span<const size_t> entries,
                       const std::function<bool(int group, int number)>& itemExists, const std::string& source,
                       std::vector<ItemDataIssue>& issues)
{
    for (size_t position = 0; position < buffs.size(); ++position)
    {
        const BuffDefinition& buff = buffs[position];
        if (!buff.item || itemExists(buff.item->group, buff.item->number))
        {
            continue;
        }
        const size_t entry = position < entries.size() ? entries[position] : position;
        AddIssue(issues, ItemDataIssueSeverity::Error, source, BuffField(entry, Keys::Item),
                 "names item (" + std::to_string(buff.item->group) + ", " + std::to_string(buff.item->number) +
                     "), which the item list does not have; the buff gets no duration or values from it");
    }
}
} // namespace Data::Buffs
