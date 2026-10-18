#include "stdafx.h"

#include "ItemStatsJson.h"
#include "ItemJsonCommon.h"

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
} // namespace Keys

constexpr const char* NoneValue = "none";

// Same order as ItemStat.
constexpr std::array<const char*, ItemStatCount> StatKeys = {"damage", "magicPower", "blockRate", "defense",
                                                             "magicDefense"};

constexpr int MaxValue = 65535;
constexpr int MaxPerLevelAfterHalving = 255;
constexpr int MaxDropLevelBonus = 1000;

using KeySet = std::set<std::string, std::less<>>;

const KeySet RootKeys = {Keys::FormatVersion, Keys::DropLevelBonus, Keys::Tables, Keys::Default, Keys::Profiles};
const KeySet DropLevelBonusKeys = {Keys::Ancient};
const KeySet TableKeys = {Keys::Name, Keys::Caption, Keys::Values, Keys::PerLevel};
const KeySet SectionKeys = {Keys::Table, Keys::Excellent, Keys::Ancient};
const KeySet MagicPowerSectionKeys = {Keys::Table, Keys::Excellent, Keys::Ancient, Keys::PerLevelAfterHalving};
const KeySet ExcellentTermKeys = {Keys::Times, Keys::Add, Keys::DropLevelDivisor};
const KeySet AncientTermKeys = {Keys::Times, Keys::Add, Keys::DropLevelDivisor, Keys::AfterLevelBonus};

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

    void AddError(const std::string& field, const std::string& message)
    {
        AddIssue(ItemDataIssueSeverity::Error, field, message);
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
    void WarnAboutUnusedTables(const ItemStatFile& file);

    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

void ItemStatsReader::WarnAboutUnknownKeys(const OrderedJson& json, const KeySet& knownKeys, const std::string& prefix)
{
    for (const auto& [key, value] : json.items())
    {
        if (!knownKeys.contains(key))
        {
            AddIssue(ItemDataIssueSeverity::Warning, prefix + key, "unknown field; it is ignored");
        }
    }
}

bool ItemStatsReader::ReadNumber(const OrderedJson& json, const std::string& field, int minimum, int maximum,
                                 int& value)
{
    long long number = 0;
    if (!json.is_number_integer() || !Json::ReadWholeNumber(json, number) || number < minimum || number > maximum)
    {
        AddError(field, "must be a whole number from " + std::to_string(minimum) + " to " + std::to_string(maximum));
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
        AddError(field + "." + Keys::Name, "missing or not a name of letters and digits");
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
    WarnAboutUnusedTables(file);
}

void ItemStatsReader::ReadDropLevelBonus(const OrderedJson& root, ItemStatFile& file)
{
    const auto bonus = root.find(Keys::DropLevelBonus);
    if (bonus == root.end() || !bonus->is_object())
    {
        AddError(Keys::DropLevelBonus, "missing or not an object");
        return;
    }
    const std::string prefix = std::string(Keys::DropLevelBonus) + ".";
    WarnAboutUnknownKeys(*bonus, DropLevelBonusKeys, prefix);
    const auto ancient = bonus->find(Keys::Ancient);
    if (ancient == bonus->end())
    {
        AddError(prefix + Keys::Ancient, "missing");
        return;
    }
    ReadNumber(*ancient, prefix + Keys::Ancient, 0, MaxDropLevelBonus, file.ancientDropLevelBonus);
}

void ItemStatsReader::ReadTables(const OrderedJson& root, ItemStatFile& file)
{
    const auto tables = root.find(Keys::Tables);
    if (tables == root.end() || !tables->is_array())
    {
        AddError(Keys::Tables, "missing or not a list");
        return;
    }
    std::set<std::string, std::less<>> names;
    for (size_t index = 0; index < tables->size(); ++index)
    {
        const std::string field = std::string(Keys::Tables) + "[" + std::to_string(index) + "]";
        const OrderedJson& json = (*tables)[index];
        if (!json.is_object())
        {
            AddError(field, "a table must be an object");
            continue;
        }
        LevelBonusTable table;
        if (!ReadName(json, field, table.name))
        {
            continue;
        }
        if (!names.insert(table.name).second)
        {
            AddError(field + "." + Keys::Name, "\"" + table.name + "\" is defined more than once");
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
        AddError(field, "needs \"values\" or \"perLevel\"");
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
    if (json.is_string())
    {
        caption.Set(LocalizedString::NeutralLocale, json.get<std::string>());
        return;
    }
    if (!json.is_object())
    {
        AddError(field, "must be a text or an object of texts by language");
        return;
    }
    for (const auto& [locale, text] : json.items())
    {
        if (!text.is_string())
        {
            AddError(field + "." + locale, "must be a text");
            continue;
        }
        caption.Set(locale, text.get<std::string>());
    }
    if (caption.GetNeutral().empty())
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
        [this](const std::string& errorField, const std::string& message) { AddError(errorField, message); });
}

void ItemStatsReader::ReadDefaults(const OrderedJson& root, ItemStatFile& file)
{
    const auto defaults = root.find(Keys::Default);
    if (defaults == root.end() || !defaults->is_object())
    {
        AddError(Keys::Default, "missing or not an object");
        return;
    }
    const KeySet statKeys(StatKeys.begin(), StatKeys.end());
    WarnAboutUnknownKeys(*defaults, statKeys, std::string(Keys::Default) + ".");
    for (size_t stat = 0; stat < ItemStatCount; ++stat)
    {
        const std::string field = std::string(Keys::Default) + "." + StatKeys[stat];
        const auto section = defaults->find(StatKeys[stat]);
        if (section == defaults->end())
        {
            AddError(field, "missing");
            continue;
        }
        ReadSection(*section, field, static_cast<ItemStat>(stat), true, file, file.defaults[stat]);
    }
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
        AddError(Keys::Profiles, "must be a list");
        return;
    }
    KeySet profileKeys(StatKeys.begin(), StatKeys.end());
    profileKeys.insert(Keys::Name);
    std::set<std::string, std::less<>> names;
    for (size_t index = 0; index < profiles->size(); ++index)
    {
        const std::string position = std::string(Keys::Profiles) + "[" + std::to_string(index) + "]";
        const OrderedJson& json = (*profiles)[index];
        if (!json.is_object())
        {
            AddError(position, "a profile must be an object");
            continue;
        }
        WrittenStatProfile profile;
        if (!ReadName(json, position, profile.name))
        {
            continue;
        }
        if (!names.insert(profile.name).second)
        {
            AddError(position + "." + Keys::Name, "\"" + profile.name + "\" is defined more than once");
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
        file.profiles.push_back(std::move(profile));
    }
}

void ItemStatsReader::ReadSection(const OrderedJson& json, const std::string& field, ItemStat stat, bool isDefault,
                                  const ItemStatFile& file, WrittenStatSection& section)
{
    if (!json.is_object())
    {
        AddError(field, "must be an object");
        return;
    }
    const bool magicPower = stat == ItemStat::MagicPower;
    WarnAboutUnknownKeys(json, magicPower ? MagicPowerSectionKeys : SectionKeys, field + ".");

    const auto missing = [&](const char* key)
    {
        if (isDefault && !json.contains(key))
        {
            AddError(field + "." + key, "missing; the default writes every term");
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
            AddError(tableField, "must name a table of \"tables\"");
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
        AddError(field, std::string("must be an object or \"") + NoneValue + "\"");
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
                AddError(field + "." + Keys::AfterLevelBonus, "must be true or false");
            }
            else
            {
                term.values.afterLevelBonus = after->get<bool>();
            }
        }
    }
}

void ItemStatsReader::WarnAboutUnusedTables(const ItemStatFile& file)
{
    std::set<std::string, std::less<>> used;
    const auto collect = [&used](const std::array<WrittenStatSection, ItemStatCount>& sections)
    {
        for (const WrittenStatSection& section : sections)
        {
            if (section.table)
            {
                used.insert(*section.table);
            }
        }
    };
    collect(file.defaults);
    for (const WrittenStatProfile& profile : file.profiles)
    {
        collect(profile.stats);
    }
    for (const LevelBonusTable& table : file.tables)
    {
        if (!used.contains(table.name))
        {
            AddIssue(ItemDataIssueSeverity::Warning, std::string(Keys::Tables) + "." + table.name,
                     "no stat uses this table");
        }
    }
}

// ---------------------------------------------------------------- writing

// English first, then the translations sorted by language.
OrderedJson WriteCaption(const LocalizedString& caption)
{
    OrderedJson json = OrderedJson::object();
    json[std::string(LocalizedString::NeutralLocale)] = caption.GetNeutral();
    for (const auto& [locale, text] : caption.GetTranslations())
    {
        json[locale] = text;
    }
    return json;
}

OrderedJson WriteTable(const LevelBonusTable& table)
{
    OrderedJson json;
    json[Keys::Name] = table.name;
    if (!table.caption.IsEmpty())
    {
        json[Keys::Caption] = WriteCaption(table.caption);
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
        root[Keys::Profiles].push_back(std::move(json));
    }

    std::string text = root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    text = Json::PutObjectsOnOneLine(text, Keys::DropLevelBonus);
    text = Json::PutObjectsOnOneLine(text, Keys::Caption);
    for (const char* statKey : StatKeys)
    {
        text = Json::PutObjectsOnOneLine(text, statKey);
    }
    text = Json::PutListsOnOneLine(text, Keys::Values);
    return text + "\n";
}
} // namespace Data::Items
