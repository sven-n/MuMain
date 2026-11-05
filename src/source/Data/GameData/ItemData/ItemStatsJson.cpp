#include "stdafx.h"

#include "ItemStatsJson.h"
#include "ItemEnumNames.h"
#include "ItemJsonCommon.h"
#include "ItemStatProfiles.h"

#include <algorithm>
#include <array>
#include <set>

namespace Data::Items
{
namespace
{
using Json::OrderedJson;

namespace Keys
{
constexpr const char* FormatVersion = "formatVersion";
constexpr const char* DropLevelBonus = "dropLevelBonus";
constexpr const char* Ancient = "ancient";
constexpr const char* Tables = "tables";
constexpr const char* Name = "name";
constexpr const char* Caption = "caption";
constexpr const char* Values = "values";
constexpr const char* PerLevel = "perLevel";
constexpr const char* Default = "default";
constexpr const char* Profiles = "profiles";
constexpr const char* Table = "table";
constexpr const char* Excellent = "excellent";
constexpr const char* PerLevelAfterHalving = "perLevelAfterHalving";
constexpr const char* Times = "times";
constexpr const char* Add = "add";
constexpr const char* DropLevelDivisor = "dropLevelDivisor";
constexpr const char* AfterLevelBonus = "afterLevelBonus";
constexpr const char* Requirements = "requirements";
constexpr const char* LevelPerItemLevel = "levelPerItemLevel";
constexpr const char* ExcellentLevelAdd = "excellentLevelAdd";
constexpr const char* StrengthPerOptionLevel = "strengthPerOptionLevel";
constexpr const char* LevelWeight = "levelWeight";
constexpr const char* Percent = "percent";
constexpr const char* Basis = "basis";
constexpr const char* ProfileRules = "profileRules";
constexpr const char* Profile = "profile";
constexpr const char* Group = "group";
constexpr const char* Slot = "slot";
constexpr const char* WingTier = "wingTier";
constexpr const char* WeaponKind = "weaponKind";
constexpr const char* Tags = "tags";
} // namespace Keys

constexpr const char* NoneValue = "none";
constexpr const char* BaseValue = "base";
constexpr const char* LevelRequirementBasis = "levelRequirement";

// Same order as ItemRequirementStat.
constexpr std::array<const char*, ItemRequirementStatCount> RequirementStatKeys = {"strength", "dexterity", "energy",
                                                                                   "vitality", "leadership"};

// Same order as ItemStat.
constexpr std::array<const char*, ItemStatCount> StatKeys = {"damage", "magicPower", "blockRate", "defense",
                                                             "magicDefense"};

constexpr int MaxValue = 65535;
constexpr int MaxPerLevelAfterHalving = 255;
constexpr int MaxStrengthPerOptionLevel = 255;
constexpr int MaxDropLevelBonus = 1000;

using Json::KeySet;

const KeySet RootKeys = {Keys::FormatVersion, Keys::DropLevelBonus, Keys::Tables,
                         Keys::Default,       Keys::Profiles,       Keys::ProfileRules};
const KeySet DropLevelBonusKeys = {Keys::Excellent, Keys::Ancient};
const KeySet TableKeys = {Keys::Name, Keys::Caption, Keys::Values, Keys::PerLevel};
const KeySet SectionKeys = {Keys::Table, Keys::Excellent, Keys::Ancient};
const KeySet MagicPowerSectionKeys = {Keys::Table, Keys::Excellent, Keys::Ancient, Keys::PerLevelAfterHalving};
const KeySet ExcellentTermKeys = {Keys::Times, Keys::Add, Keys::DropLevelDivisor};
const KeySet AncientTermKeys = {Keys::Times, Keys::Add, Keys::DropLevelDivisor, Keys::AfterLevelBonus};
const KeySet RequirementKeys = []
{
    KeySet keys(RequirementStatKeys.begin(), RequirementStatKeys.end());
    keys.insert({Keys::LevelPerItemLevel, Keys::ExcellentLevelAdd, Keys::StrengthPerOptionLevel});
    return keys;
}();
const KeySet RequirementFormulaKeys = {Keys::Add, Keys::LevelWeight, Keys::Percent, Keys::Basis};

// ---------------------------------------------------------------- reading

class ItemStatsReader
{
public:
    ItemStatsReader(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues)
    {
    }

    void Read(const OrderedJson& root, ItemStatFile& file);

private:
    void AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    {
        m_issues.push_back({severity, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
    }

    void AddFatal(const std::string& field, const std::string& message)
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field, message);
    }

    void WarnAboutUnknownKeys(const OrderedJson& json, const KeySet& knownKeys, const std::string& prefix);
    bool ReadNumber(const OrderedJson& json, const std::string& field, int minimum, int maximum, int& value);
    bool ReadName(const OrderedJson& json, const std::string& field, std::string& name);
    void ReadDropLevelBonus(const OrderedJson& root, ItemStatFile& file);
    void ReadTables(const OrderedJson& root, ItemStatFile& file);
    void ReadTable(const OrderedJson& json, const std::string& field, LevelBonusTable& table);
    void ReadCaption(const OrderedJson& json, const std::string& field, LocalizedString& caption);
    void ReadValues(const OrderedJson& json, const std::string& field, std::array<int, ItemLevelCount>& values);
    void ReadDefaults(const OrderedJson& root, ItemStatFile& file);
    void ReadProfiles(const OrderedJson& root, ItemStatFile& file);
    void ReadSection(const OrderedJson& json, const std::string& field, ItemStat stat, bool isDefault,
                     const ItemStatFile& file, WrittenStatSection& section);
    void ReadTerm(const OrderedJson& json, const std::string& field, bool ancient, WrittenStatTerm& term);
    void ReadRequirements(const OrderedJson& json, const std::string& field, bool isDefault,
                          WrittenRequirementSection& section);
    void ReadRequirementFormula(const OrderedJson& json, const std::string& field,
                                std::optional<RequirementFormula>& formula);
    void ReadProfileRules(const OrderedJson& root, ItemStatFile& file);
    bool ReadRule(const OrderedJson& json, const std::string& field, const ItemStatFile& file, StatProfileRule& rule);
    // A condition's list of values: the names of `TEnum`, `what` being their kind
    // ("slot") for messages.
    template <typename TEnum, typename TSet>
    bool ReadCondition(const OrderedJson& json, const std::string& field, const char* what, TSet& values);
    bool ReadGroups(const OrderedJson& json, const std::string& field, std::bitset<MAX_ITEM_TYPE>& groups);

    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

void ItemStatsReader::WarnAboutUnknownKeys(const OrderedJson& json, const KeySet& knownKeys, const std::string& prefix)
{
    Json::WarnAboutUnknownKeys(json, knownKeys, prefix, [this](const std::string& field, const std::string& message)
                               { AddIssue(ItemDataIssueSeverity::Warning, field, message); });
}

bool ItemStatsReader::ReadNumber(const OrderedJson& json, const std::string& field, int minimum, int maximum,
                                 int& value)
{
    long long number = 0;
    if (!json.is_number_integer() || !Json::ReadWholeNumber(json, number) || number < minimum || number > maximum)
    {
        AddFatal(field, "must be a whole number from " + std::to_string(minimum) + " to " + std::to_string(maximum));
        return false;
    }
    value = static_cast<int>(number);
    return true;
}

bool ItemStatsReader::ReadName(const OrderedJson& json, const std::string& field, std::string& name)
{
    const auto value = json.find(Keys::Name);
    if (value == json.end() || !value->is_string() || !Json::IsName(value->get_ref<const std::string&>()))
    {
        AddFatal(field + "." + Keys::Name, "missing or not a name of letters and digits");
        return false;
    }
    name = value->get<std::string>();
    return true;
}

void ItemStatsReader::Read(const OrderedJson& root, ItemStatFile& file)
{
    WarnAboutUnknownKeys(root, RootKeys, "");
    ReadDropLevelBonus(root, file);
    ReadTables(root, file);
    ReadDefaults(root, file);
    ReadProfiles(root, file);
    ReadProfileRules(root, file);
}

void ItemStatsReader::ReadDropLevelBonus(const OrderedJson& root, ItemStatFile& file)
{
    const auto bonus = root.find(Keys::DropLevelBonus);
    if (bonus == root.end() || !bonus->is_object())
    {
        AddFatal(Keys::DropLevelBonus, "missing or not an object");
        return;
    }
    const std::string prefix = std::string(Keys::DropLevelBonus) + ".";
    WarnAboutUnknownKeys(*bonus, DropLevelBonusKeys, prefix);
    const auto read = [&](const char* key, int& value)
    {
        const auto field = bonus->find(key);
        if (field == bonus->end())
        {
            AddFatal(prefix + key, "missing");
            return;
        }
        ReadNumber(*field, prefix + key, 0, MaxDropLevelBonus, value);
    };
    read(Keys::Excellent, file.excellentDropLevelBonus);
    read(Keys::Ancient, file.ancientDropLevelBonus);
}

void ItemStatsReader::ReadTables(const OrderedJson& root, ItemStatFile& file)
{
    const auto tables = root.find(Keys::Tables);
    if (tables == root.end() || !tables->is_array())
    {
        AddFatal(Keys::Tables, "missing or not a list");
        return;
    }
    std::set<std::string, std::less<>> names;
    for (size_t index = 0; index < tables->size(); ++index)
    {
        const std::string field = std::string(Keys::Tables) + "[" + std::to_string(index) + "]";
        const OrderedJson& json = (*tables)[index];
        if (!json.is_object())
        {
            AddFatal(field, "a table must be an object");
            continue;
        }
        LevelBonusTable table;
        if (!ReadName(json, field, table.name))
        {
            continue;
        }
        if (!names.insert(table.name).second)
        {
            AddFatal(field + "." + Keys::Name, "\"" + table.name + "\" is defined more than once");
            continue;
        }
        ReadTable(json, std::string(Keys::Tables) + "." + table.name, table);
        file.tables.push_back(std::move(table));
    }
}

void ItemStatsReader::ReadTable(const OrderedJson& json, const std::string& field, LevelBonusTable& table)
{
    WarnAboutUnknownKeys(json, TableKeys, field + ".");
    if (const auto caption = json.find(Keys::Caption); caption != json.end())
    {
        ReadCaption(*caption, field + "." + Keys::Caption, table.caption);
    }
    const auto values = json.find(Keys::Values);
    const auto perLevel = json.find(Keys::PerLevel);
    if (values == json.end() && perLevel == json.end())
    {
        AddFatal(field, "needs \"values\" or \"perLevel\"");
        return;
    }
    if (values != json.end())
    {
        ReadValues(*values, field + "." + Keys::Values, table.values);
    }
    if (perLevel != json.end())
    {
        ReadNumber(*perLevel, field + "." + Keys::PerLevel, 0, MaxValue, table.perLevel);
    }
}

void ItemStatsReader::ReadCaption(const OrderedJson& json, const std::string& field, LocalizedString& caption)
{
    const bool read = Json::ReadLocalizedString(json, field, caption,
                                                [this](const std::string& errorField, const std::string& message)
                                                { AddFatal(errorField, message); });
    if (read && caption.GetNeutral().empty())
    {
        AddIssue(ItemDataIssueSeverity::Warning, field, "has no English text");
    }
}

void ItemStatsReader::ReadValues(const OrderedJson& json, const std::string& field,
                                 std::array<int, ItemLevelCount>& values)
{
    Json::ReadPerLevelValues(
        json, field, values, [this](const OrderedJson& entry, const std::string& entryField, int& value)
        { ReadNumber(entry, entryField, 0, MaxValue, value); },
        [this](const std::string& errorField, const std::string& message) { AddFatal(errorField, message); });
}

void ItemStatsReader::ReadDefaults(const OrderedJson& root, ItemStatFile& file)
{
    const auto defaults = root.find(Keys::Default);
    if (defaults == root.end() || !defaults->is_object())
    {
        AddFatal(Keys::Default, "missing or not an object");
        return;
    }
    KeySet defaultKeys(StatKeys.begin(), StatKeys.end());
    defaultKeys.insert(Keys::Requirements);
    WarnAboutUnknownKeys(*defaults, defaultKeys, std::string(Keys::Default) + ".");
    for (size_t stat = 0; stat < ItemStatCount; ++stat)
    {
        const std::string field = std::string(Keys::Default) + "." + StatKeys[stat];
        const auto section = defaults->find(StatKeys[stat]);
        if (section == defaults->end())
        {
            AddFatal(field, "missing");
            continue;
        }
        ReadSection(*section, field, static_cast<ItemStat>(stat), true, file, file.defaults[stat]);
    }
    const std::string requirementsField = std::string(Keys::Default) + "." + Keys::Requirements;
    const auto requirements = defaults->find(Keys::Requirements);
    if (requirements == defaults->end())
    {
        AddFatal(requirementsField, "missing");
        return;
    }
    ReadRequirements(*requirements, requirementsField, true, file.defaultRequirements);
}

void ItemStatsReader::ReadProfiles(const OrderedJson& root, ItemStatFile& file)
{
    const auto profiles = root.find(Keys::Profiles);
    if (profiles == root.end())
    {
        return;
    }
    if (!profiles->is_array())
    {
        AddFatal(Keys::Profiles, "must be a list");
        return;
    }
    KeySet profileKeys(StatKeys.begin(), StatKeys.end());
    profileKeys.insert(Keys::Name);
    profileKeys.insert(Keys::Requirements);
    std::set<std::string, std::less<>> names;
    for (size_t index = 0; index < profiles->size(); ++index)
    {
        const std::string position = std::string(Keys::Profiles) + "[" + std::to_string(index) + "]";
        const OrderedJson& json = (*profiles)[index];
        if (!json.is_object())
        {
            AddFatal(position, "a profile must be an object");
            continue;
        }
        WrittenStatProfile profile;
        if (!ReadName(json, position, profile.name))
        {
            continue;
        }
        if (profile.name == StatProfileNames::Default)
        {
            AddFatal(position + "." + Keys::Name, "\"" + profile.name + "\" names the default, not a profile");
            continue;
        }
        if (!names.insert(profile.name).second)
        {
            AddFatal(position + "." + Keys::Name, "\"" + profile.name + "\" is defined more than once");
            continue;
        }
        const std::string field = std::string(Keys::Profiles) + "." + profile.name;
        WarnAboutUnknownKeys(json, profileKeys, field + ".");
        for (size_t stat = 0; stat < ItemStatCount; ++stat)
        {
            if (const auto section = json.find(StatKeys[stat]); section != json.end())
            {
                ReadSection(*section, field + "." + StatKeys[stat], static_cast<ItemStat>(stat), false, file,
                            profile.stats[stat]);
            }
        }
        if (const auto requirements = json.find(Keys::Requirements); requirements != json.end())
        {
            ReadRequirements(*requirements, field + "." + Keys::Requirements, false, profile.requirements);
        }
        file.profiles.push_back(std::move(profile));
    }
}

void ItemStatsReader::ReadSection(const OrderedJson& json, const std::string& field, ItemStat stat, bool isDefault,
                                  const ItemStatFile& file, WrittenStatSection& section)
{
    if (!json.is_object())
    {
        AddFatal(field, "must be an object");
        return;
    }
    const bool magicPower = stat == ItemStat::MagicPower;
    WarnAboutUnknownKeys(json, magicPower ? MagicPowerSectionKeys : SectionKeys, field + ".");

    const auto missing = [&](const char* key)
    {
        if (isDefault && !json.contains(key))
        {
            AddFatal(field + "." + key, "missing; the default writes every term");
        }
    };
    missing(Keys::Table);
    missing(Keys::Excellent);
    missing(Keys::Ancient);
    if (magicPower)
    {
        missing(Keys::PerLevelAfterHalving);
    }

    if (const auto table = json.find(Keys::Table); table != json.end())
    {
        const std::string tableField = field + "." + Keys::Table;
        const bool known =
            table->is_string() && std::any_of(file.tables.begin(), file.tables.end(), [&](const LevelBonusTable& entry)
                                              { return entry.name == table->get_ref<const std::string&>(); });
        if (!known)
        {
            AddFatal(tableField, "must name a table of \"tables\"");
        }
        else
        {
            section.table = table->get<std::string>();
        }
    }
    if (const auto excellent = json.find(Keys::Excellent); excellent != json.end())
    {
        ReadTerm(*excellent, field + "." + Keys::Excellent, false, section.excellent);
    }
    if (const auto ancient = json.find(Keys::Ancient); ancient != json.end())
    {
        ReadTerm(*ancient, field + "." + Keys::Ancient, true, section.ancient);
    }
    if (magicPower)
    {
        if (const auto perLevel = json.find(Keys::PerLevelAfterHalving); perLevel != json.end())
        {
            int value = 0;
            if (ReadNumber(*perLevel, field + "." + Keys::PerLevelAfterHalving, 0, MaxPerLevelAfterHalving, value))
            {
                section.perLevelAfterHalving = value;
            }
        }
    }
}

void ItemStatsReader::ReadTerm(const OrderedJson& json, const std::string& field, bool ancient, WrittenStatTerm& term)
{
    if (json.is_string() && json.get_ref<const std::string&>() == NoneValue)
    {
        term.state = WrittenStatTerm::State::None;
        return;
    }
    if (!json.is_object())
    {
        AddFatal(field, std::string("must be an object or \"") + NoneValue + "\"");
        return;
    }
    WarnAboutUnknownKeys(json, ancient ? AncientTermKeys : ExcellentTermKeys, field + ".");
    term.state = WrittenStatTerm::State::Values;
    if (const auto times = json.find(Keys::Times); times != json.end())
    {
        ReadNumber(*times, field + "." + Keys::Times, 0, MaxValue, term.values.times);
    }
    if (const auto add = json.find(Keys::Add); add != json.end())
    {
        ReadNumber(*add, field + "." + Keys::Add, 0, MaxValue, term.values.add);
    }
    if (const auto divisor = json.find(Keys::DropLevelDivisor); divisor != json.end())
    {
        // Left out, the part is not added; 0 would divide by zero.
        ReadNumber(*divisor, field + "." + Keys::DropLevelDivisor, 1, MaxValue, term.values.dropLevelDivisor);
    }
    if (ancient)
    {
        if (const auto after = json.find(Keys::AfterLevelBonus); after != json.end())
        {
            if (!after->is_boolean())
            {
                AddFatal(field + "." + Keys::AfterLevelBonus, "must be true or false");
            }
            else
            {
                term.values.afterLevelBonus = after->get<bool>();
            }
        }
    }
}

void ItemStatsReader::ReadRequirements(const OrderedJson& json, const std::string& field, bool isDefault,
                                       WrittenRequirementSection& section)
{
    if (!json.is_object())
    {
        AddFatal(field, "must be an object");
        return;
    }
    WarnAboutUnknownKeys(json, RequirementKeys, field + ".");

    const auto readNumber = [&](const char* key, int maximum, std::optional<int>& value)
    {
        const auto entry = json.find(key);
        if (entry == json.end())
        {
            if (isDefault)
            {
                AddFatal(field + "." + key, "missing; the default writes every term");
            }
            return;
        }
        int number = 0;
        if (ReadNumber(*entry, field + "." + key, 0, maximum, number))
        {
            value = number;
        }
    };
    readNumber(Keys::LevelPerItemLevel, MaxValue, section.levelPerItemLevel);
    readNumber(Keys::ExcellentLevelAdd, MaxValue, section.excellentLevelAdd);
    for (size_t stat = 0; stat < ItemRequirementStatCount; ++stat)
    {
        const std::string statField = field + "." + RequirementStatKeys[stat];
        const auto entry = json.find(RequirementStatKeys[stat]);
        if (entry == json.end())
        {
            if (isDefault)
            {
                AddFatal(statField, "missing; the default writes every term");
            }
            continue;
        }
        ReadRequirementFormula(*entry, statField, section.stats[stat]);
    }
    readNumber(Keys::StrengthPerOptionLevel, MaxStrengthPerOptionLevel, section.strengthPerOptionLevel);
}

void ItemStatsReader::ReadRequirementFormula(const OrderedJson& json, const std::string& field,
                                             std::optional<RequirementFormula>& formula)
{
    if (json.is_string() && json.get_ref<const std::string&>() == BaseValue)
    {
        formula = RequirementFormula{};
        return;
    }
    if (!json.is_object())
    {
        AddFatal(field, std::string("must be an object or \"") + BaseValue + "\"");
        return;
    }
    WarnAboutUnknownKeys(json, RequirementFormulaKeys, field + ".");
    RequirementFormula values;
    values.keepBase = false;
    if (const auto add = json.find(Keys::Add); add != json.end())
    {
        ReadNumber(*add, field + "." + Keys::Add, 0, MaxValue, values.add);
    }
    if (const auto levelWeight = json.find(Keys::LevelWeight); levelWeight != json.end())
    {
        ReadNumber(*levelWeight, field + "." + Keys::LevelWeight, 0, MaxValue, values.levelWeight);
    }
    if (const auto percent = json.find(Keys::Percent); percent != json.end())
    {
        ReadNumber(*percent, field + "." + Keys::Percent, 0, MaxValue, values.percent);
    }
    if (const auto basis = json.find(Keys::Basis); basis != json.end())
    {
        if (!basis->is_string() || basis->get_ref<const std::string&>() != LevelRequirementBasis)
        {
            AddFatal(field + "." + Keys::Basis, std::string("must be \"") + LevelRequirementBasis + "\"");
        }
        else
        {
            values.levelRequirementBasis = true;
        }
    }
    formula = values;
}

void AddValue(ItemTagSet& values, ItemTag tag)
{
    values.Set(tag);
}

template <size_t Size, typename TEnum> void AddValue(std::bitset<Size>& values, TEnum value)
{
    values.set(static_cast<size_t>(value));
}

template <typename TEnum, typename TSet>
bool ItemStatsReader::ReadCondition(const OrderedJson& json, const std::string& field, const char* what, TSet& values)
{
    if (!json.is_array() || json.empty())
    {
        AddFatal(field, std::string("must be a list of one or more ") + what + "s");
        return false;
    }
    bool valid = true;
    for (size_t index = 0; index < json.size(); ++index)
    {
        const OrderedJson& entry = json[index];
        TEnum value{};
        if (!entry.is_string() || !FindEnumValue(entry.get_ref<const std::string&>(), value))
        {
            AddFatal(field + "[" + std::to_string(index) + "]",
                     entry.dump() + " is not a " + what + "; " + what + "s are " + JoinEnumNames<TEnum>());
            valid = false;
            continue;
        }
        AddValue(values, value);
    }
    return valid;
}

bool ItemStatsReader::ReadGroups(const OrderedJson& json, const std::string& field, std::bitset<MAX_ITEM_TYPE>& groups)
{
    if (!json.is_array() || json.empty())
    {
        AddFatal(field, "must be a list of one or more groups");
        return false;
    }
    bool valid = true;
    for (size_t index = 0; index < json.size(); ++index)
    {
        int group = 0;
        if (!ReadNumber(json[index], field + "[" + std::to_string(index) + "]", 0, MAX_ITEM_TYPE - 1, group))
        {
            valid = false;
            continue;
        }
        groups.set(static_cast<size_t>(group));
    }
    return valid;
}

bool ItemStatsReader::ReadRule(const OrderedJson& json, const std::string& field, const ItemStatFile& file,
                               StatProfileRule& rule)
{
    bool valid = true;
    const auto profile = json.find(Keys::Profile);
    if (profile == json.end() || !profile->is_string() || !Json::IsName(profile->get_ref<const std::string&>()))
    {
        AddFatal(field + "." + Keys::Profile, "missing or not a name of letters and digits");
        valid = false;
    }
    else
    {
        rule.profile = profile->get<std::string>();
        const bool known = rule.profile == StatProfileNames::Default ||
                           std::any_of(file.profiles.begin(), file.profiles.end(),
                                       [&rule](const WrittenStatProfile& p) { return p.name == rule.profile; });
        if (!known)
        {
            AddFatal(field + "." + Keys::Profile, "\"" + rule.profile + "\" is not a profile of \"" + Keys::Profiles +
                                                      "\" or \"" + StatProfileNames::Default + "\"");
            valid = false;
        }
    }
    // An unknown condition is an error: leaving it out would widen the rule.
    for (const auto& [key, value] : json.items())
    {
        const std::string conditionField = field + "." + key;
        if (key == Keys::Profile)
        {
            continue;
        }
        bool read = true;
        if (key == Keys::Group)
        {
            read = ReadGroups(value, conditionField, rule.groups);
        }
        else if (key == Keys::Slot)
        {
            read = ReadCondition<ItemSlot>(value, conditionField, "slot", rule.slots);
        }
        else if (key == Keys::WingTier)
        {
            read = ReadCondition<WingTier>(value, conditionField, "wing tier", rule.wingTiers);
        }
        else if (key == Keys::WeaponKind)
        {
            read = ReadCondition<WeaponKind>(value, conditionField, "weapon kind", rule.weaponKinds);
        }
        else if (key == Keys::Tags)
        {
            read = ReadCondition<ItemTag>(value, conditionField, "tag", rule.tags);
        }
        else if (key == Keys::Requirements)
        {
            read = ReadCondition<ItemRequirement>(value, conditionField, "requirement", rule.requirements);
        }
        else
        {
            AddFatal(conditionField, std::string("is not a condition; the conditions are \"") + Keys::Group + "\", \"" +
                                         Keys::Slot + "\", \"" + Keys::WingTier + "\", \"" + Keys::WeaponKind +
                                         "\", \"" + Keys::Tags + "\" and \"" + Keys::Requirements + "\"");
            read = false;
        }
        valid = valid && read;
    }
    return valid;
}

void ItemStatsReader::ReadProfileRules(const OrderedJson& root, ItemStatFile& file)
{
    const auto rules = root.find(Keys::ProfileRules);
    if (rules == root.end())
    {
        return;
    }
    if (!rules->is_array())
    {
        AddFatal(Keys::ProfileRules, "must be a list");
        return;
    }
    for (size_t index = 0; index < rules->size(); ++index)
    {
        const std::string field = std::string(Keys::ProfileRules) + "[" + std::to_string(index) + "]";
        const OrderedJson& json = (*rules)[index];
        if (!json.is_object())
        {
            AddFatal(field, "a rule must be an object");
            continue;
        }
        StatProfileRule rule;
        if (ReadRule(json, field, file, rule))
        {
            file.profileRules.push_back(std::move(rule));
        }
    }
}

// ---------------------------------------------------------------- writing

OrderedJson WriteTable(const LevelBonusTable& table)
{
    OrderedJson json;
    json[Keys::Name] = table.name;
    if (!table.caption.IsEmpty())
    {
        json[Keys::Caption] = Json::WriteLocalizedString(table.caption);
    }
    // A table that only grows per level needs no values.
    const bool noValues = std::all_of(table.values.begin(), table.values.end(), [](int value) { return value == 0; });
    if (!(noValues && table.perLevel != 0))
    {
        json[Keys::Values] = Json::WritePerLevelValues(table.values);
    }
    if (table.perLevel != 0)
    {
        json[Keys::PerLevel] = table.perLevel;
    }
    return json;
}

OrderedJson WriteTerm(const WrittenStatTerm& term)
{
    if (term.state == WrittenStatTerm::State::None)
    {
        return NoneValue;
    }
    OrderedJson json = OrderedJson::object();
    if (term.values.times != 0)
    {
        json[Keys::Times] = term.values.times;
    }
    if (term.values.add != 0)
    {
        json[Keys::Add] = term.values.add;
    }
    if (term.values.dropLevelDivisor != 0)
    {
        json[Keys::DropLevelDivisor] = term.values.dropLevelDivisor;
    }
    if (term.values.afterLevelBonus)
    {
        json[Keys::AfterLevelBonus] = true;
    }
    return json;
}

OrderedJson WriteSection(const WrittenStatSection& section)
{
    OrderedJson json = OrderedJson::object();
    if (section.table)
    {
        json[Keys::Table] = *section.table;
    }
    if (section.excellent.state != WrittenStatTerm::State::NotWritten)
    {
        json[Keys::Excellent] = WriteTerm(section.excellent);
    }
    if (section.ancient.state != WrittenStatTerm::State::NotWritten)
    {
        json[Keys::Ancient] = WriteTerm(section.ancient);
    }
    if (section.perLevelAfterHalving)
    {
        json[Keys::PerLevelAfterHalving] = *section.perLevelAfterHalving;
    }
    return json;
}

OrderedJson WriteRequirementFormula(const RequirementFormula& formula)
{
    if (formula.keepBase)
    {
        return BaseValue;
    }
    OrderedJson json = OrderedJson::object();
    if (formula.add != 0)
    {
        json[Keys::Add] = formula.add;
    }
    if (formula.levelWeight != 0)
    {
        json[Keys::LevelWeight] = formula.levelWeight;
    }
    if (formula.percent != 0)
    {
        json[Keys::Percent] = formula.percent;
    }
    if (formula.levelRequirementBasis)
    {
        json[Keys::Basis] = LevelRequirementBasis;
    }
    return json;
}

OrderedJson WriteRequirements(const WrittenRequirementSection& section)
{
    OrderedJson json = OrderedJson::object();
    if (section.levelPerItemLevel)
    {
        json[Keys::LevelPerItemLevel] = *section.levelPerItemLevel;
    }
    if (section.excellentLevelAdd)
    {
        json[Keys::ExcellentLevelAdd] = *section.excellentLevelAdd;
    }
    for (size_t stat = 0; stat < ItemRequirementStatCount; ++stat)
    {
        if (section.stats[stat])
        {
            json[RequirementStatKeys[stat]] = WriteRequirementFormula(*section.stats[stat]);
        }
    }
    if (section.strengthPerOptionLevel)
    {
        json[Keys::StrengthPerOptionLevel] = *section.strengthPerOptionLevel;
    }
    return json;
}

bool HasValue(const ItemTagSet& values, ItemTag tag)
{
    return values.Has(tag);
}

template <size_t Size, typename TEnum> bool HasValue(const std::bitset<Size>& values, TEnum value)
{
    const auto index = static_cast<size_t>(value);
    return index < Size && values.test(index);
}

// The names of the values, in the order of the enum's names.
template <typename TEnum, typename TSet> OrderedJson WriteCondition(const TSet& values)
{
    OrderedJson json = OrderedJson::array();
    for (const EnumName<TEnum>& entry : GetEnumNames(TEnum{}))
    {
        if (HasValue(values, entry.value))
        {
            json.push_back(entry.name);
        }
    }
    return json;
}

OrderedJson WriteProfileRule(const StatProfileRule& rule)
{
    OrderedJson json;
    json[Keys::Profile] = rule.profile;
    if (rule.groups.any())
    {
        OrderedJson groups = OrderedJson::array();
        for (size_t group = 0; group < rule.groups.size(); ++group)
        {
            if (rule.groups.test(group))
            {
                groups.push_back(group);
            }
        }
        json[Keys::Group] = std::move(groups);
    }
    if (rule.slots.any())
    {
        json[Keys::Slot] = WriteCondition<ItemSlot>(rule.slots);
    }
    if (rule.wingTiers.any())
    {
        json[Keys::WingTier] = WriteCondition<WingTier>(rule.wingTiers);
    }
    if (rule.weaponKinds.any())
    {
        json[Keys::WeaponKind] = WriteCondition<WeaponKind>(rule.weaponKinds);
    }
    if (!rule.tags.IsEmpty())
    {
        json[Keys::Tags] = WriteCondition<ItemTag>(rule.tags);
    }
    if (rule.requirements.any())
    {
        json[Keys::Requirements] = WriteCondition<ItemRequirement>(rule.requirements);
    }
    return json;
}

template <typename T> std::vector<const T*> SortedByName(const std::vector<T>& entries)
{
    std::vector<const T*> sorted;
    for (const T& entry : entries)
    {
        sorted.push_back(&entry);
    }
    std::sort(sorted.begin(), sorted.end(), [](const T* left, const T* right) { return left->name < right->name; });
    return sorted;
}
} // namespace

void ReadItemStatsJson(std::string_view text, const std::string& source, ItemStatFile& file,
                       std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, ItemStatsJsonFormatVersion, root, issues))
    {
        return;
    }
    ItemStatsReader(source, issues).Read(root, file);
}

std::string WriteItemStatsJson(const ItemStatFile& file)
{
    OrderedJson root;
    root[Keys::FormatVersion] = ItemStatsJsonFormatVersion;
    OrderedJson formulas = WriteItemStatFormulasJson(file);
    for (auto& [key, value] : formulas.items())
    {
        root[key] = std::move(value);
    }
    root[Keys::ProfileRules] = OrderedJson::array();
    for (const StatProfileRule& rule : file.profileRules)
    {
        root[Keys::ProfileRules].push_back(WriteProfileRule(rule));
    }
    return LayOutItemStatsText(root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace)) + "\n";
}

OrderedJson WriteItemStatFormulasJson(const ItemStatFile& file)
{
    OrderedJson root;
    root[Keys::DropLevelBonus][Keys::Excellent] = file.excellentDropLevelBonus;
    root[Keys::DropLevelBonus][Keys::Ancient] = file.ancientDropLevelBonus;

    root[Keys::Tables] = OrderedJson::array();
    for (const LevelBonusTable* table : SortedByName(file.tables))
    {
        root[Keys::Tables].push_back(WriteTable(*table));
    }

    root[Keys::Default] = OrderedJson::object();
    for (size_t stat = 0; stat < ItemStatCount; ++stat)
    {
        root[Keys::Default][StatKeys[stat]] = WriteSection(file.defaults[stat]);
    }
    root[Keys::Default][Keys::Requirements] = WriteRequirements(file.defaultRequirements);

    root[Keys::Profiles] = OrderedJson::array();
    for (const WrittenStatProfile* profile : SortedByName(file.profiles))
    {
        OrderedJson json;
        json[Keys::Name] = profile->name;
        for (size_t stat = 0; stat < ItemStatCount; ++stat)
        {
            OrderedJson section = WriteSection(profile->stats[stat]);
            if (!section.empty())
            {
                json[StatKeys[stat]] = std::move(section);
            }
        }
        OrderedJson requirements = WriteRequirements(profile->requirements);
        if (!requirements.empty())
        {
            json[Keys::Requirements] = std::move(requirements);
        }
        root[Keys::Profiles].push_back(std::move(json));
    }
    return root;
}

void ReadItemStatFormulasJson(const OrderedJson& json, const std::string& source, ItemStatFile& file,
                              std::vector<ItemDataIssue>& issues)
{
    if (!json.is_object())
    {
        Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, "", "the formulas must be an object");
        return;
    }
    ItemStatsReader(source, issues).Read(json, file);
    if (json.contains(Keys::ProfileRules))
    {
        issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                          Keys::ProfileRules, "the profile rules stay with the client, ignored"});
        file.profileRules.clear();
    }
}

std::string LayOutItemStatsText(std::string text)
{
    text = Json::PutListEntriesOnOneLine(text, Keys::ProfileRules);
    text = Json::PutObjectsOnOneLine(text, Keys::DropLevelBonus);
    text = Json::PutObjectsOnOneLine(text, Keys::Caption);
    for (const char* statKey : StatKeys)
    {
        text = Json::PutObjectsOnOneLine(text, statKey);
    }
    for (const char* requirementKey : RequirementStatKeys)
    {
        text = Json::PutObjectsOnOneLine(text, requirementKey);
    }
    text = Json::PutListsOnOneLine(text, Keys::Values);
    return text;
}
} // namespace Data::Items
