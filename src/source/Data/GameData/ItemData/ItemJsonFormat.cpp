#include "stdafx.h"

#include "ItemJsonFormat.h"
#include "ItemEnumNames.h"
#include "ItemType.h"

#include "json.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <set>
#include <type_traits>

namespace Data::Items
{
namespace
{
using OrderedJson = nlohmann::ordered_json;

constexpr int JsonIndent = 2;

// Reads a whole number without narrowing it, so a huge value cannot wrap
// around to a valid one. False when it is not a whole number or does not
// fit in a long long.
bool ReadWholeNumber(const OrderedJson& json, long long& number)
{
    if (json.is_number_unsigned())
    {
        const auto value = json.get<unsigned long long>();
        if (value > static_cast<unsigned long long>(std::numeric_limits<long long>::max()))
        {
            return false;
        }
        number = static_cast<long long>(value);
        return true;
    }
    if (json.is_number_integer())
    {
        number = json.get<long long>();
        return true;
    }
    return false;
}

namespace Keys
{
constexpr const char* FormatVersion = "formatVersion";
constexpr const char* Group = "group";
constexpr const char* Items = "items";
constexpr const char* Number = "number";
constexpr const char* Name = "name";
constexpr const char* Tags = "tags";
constexpr const char* Requirements = "requirements";
constexpr const char* ClassRequirements = "classRequirements";
constexpr const char* Resistances = "resistances";
} // namespace Keys

// Same order as ITEM_ATTRIBUTE::RequireClass.
constexpr std::array<const char*, MAX_CLASS> ClassKeys = {"darkWizard", "darkKnight", "fairyElf",   "magicGladiator",
                                                          "darkLord",   "summoner",   "rageFighter"};

// Same order as ITEM_ATTRIBUTE::Resistance. The last one is unused by the
// Season 6 data.
constexpr std::array<const char*, MAX_RESISTANCE + 1> ResistanceKeys = {"ice",   "poison", "lightning", "fire",
                                                                        "earth", "wind",   "water",     "unknown"};

// The single list of item fields and their defaults, used for reading and
// writing. TDefinition is ItemDefinition or const ItemDefinition.
template <typename TDefinition, typename TVisitor> void VisitStatFields(TDefinition& definition, TVisitor&& visit)
{
    visit("width", definition.width, BYTE{0});
    visit("height", definition.height, BYTE{0});
    visit("slot", definition.slot, ItemSlot::None);
    visit("wingTier", definition.wingTier, WingTier::None);
    visit("twoHanded", definition.twoHanded, false);
    visit("skill", definition.skill, WORD{0});
    visit("level", definition.level, WORD{0});
    visit("durability", definition.durability, BYTE{0});
    visit("magicDurability", definition.magicDurability, BYTE{0});
    visit("damageMin", definition.damageMin, BYTE{0});
    visit("damageMax", definition.damageMax, BYTE{0});
    visit("blockRate", definition.blockRate, BYTE{0});
    visit("defense", definition.defense, BYTE{0});
    visit("magicDefense", definition.magicDefense, BYTE{0});
    visit("attackSpeed", definition.attackSpeed, BYTE{0});
    visit("walkSpeed", definition.walkSpeed, BYTE{0});
    visit("magicPower", definition.magicPower, BYTE{0});
    visit("attackType", definition.attackType, BYTE{0});
    visit("sellValue", definition.sellValue, BYTE{0});
    visit("buyPrice", definition.buyPrice, 0);
    visit("tradable", definition.tradable, true);
    visit("droppable", definition.droppable, true);
    visit("storable", definition.storable, true);
    visit("sellable", definition.sellable, true);
    visit("personalShopSellable", definition.personalShopSellable, true);
    visit("repairable", definition.repairable, true);
}

// Lists the names of an enum for error messages: "a", "b" or "c".
template <typename TEnum> std::string JoinEnumNames()
{
    std::string text;
    const auto names = GetEnumNames(TEnum{});
    for (size_t i = 0; i < names.size(); ++i)
    {
        text += i == 0 ? "" : (i + 1 == names.size() ? " or " : ", ");
        text += std::string("\"") + names[i].name + "\"";
    }
    return text;
}

template <typename TRequirements, typename TVisitor>
void VisitRequirementFields(TRequirements& requirements, TVisitor&& visit)
{
    visit("level", requirements.level, WORD{0});
    visit("strength", requirements.strength, WORD{0});
    visit("dexterity", requirements.dexterity, WORD{0});
    visit("energy", requirements.energy, WORD{0});
    visit("vitality", requirements.vitality, WORD{0});
    visit("leadership", requirements.leadership, WORD{0});
}

// ---------------------------------------------------------------- writing

template <size_t Count>
void WriteByteTable(OrderedJson& item, const char* key, const std::array<BYTE, Count>& values,
                    const std::array<const char*, Count>& keys)
{
    OrderedJson table = OrderedJson::object();
    for (size_t i = 0; i < Count; ++i)
    {
        if (values[i] != 0)
        {
            table[keys[i]] = values[i];
        }
    }
    if (!table.empty())
    {
        item[key] = std::move(table);
    }
}

// Enums are written by name. A value without a name (only possible for
// data that failed validation) is written as its number.
template <typename T> OrderedJson WriteValue(const T& value)
{
    if constexpr (std::is_enum_v<T>)
    {
        const char* name = FindEnumName(value);
        return name != nullptr ? OrderedJson(name) : OrderedJson(static_cast<int>(value));
    }
    else
    {
        return OrderedJson(value);
    }
}

// Tags in the order of ItemTag.
OrderedJson WriteTags(const ItemTagSet& tags)
{
    OrderedJson json = OrderedJson::array();
    for (const EnumName<ItemTag>& tag : GetEnumNames(ItemTag{}))
    {
        if (tags.Has(tag.value))
        {
            json.push_back(tag.name);
        }
    }
    return json;
}

// English first, then the translations sorted by locale.
OrderedJson WriteNames(const LocalizedString& names)
{
    OrderedJson json = OrderedJson::object();
    json[std::string(LocalizedString::NeutralLocale)] = names.GetNeutral();
    for (const auto& [locale, text] : names.GetTranslations())
    {
        json[locale] = text;
    }
    return json;
}

OrderedJson WriteItem(const ItemDefinition& definition)
{
    OrderedJson item;
    item[Keys::Number] = definition.number;
    item[Keys::Name] = WriteNames(definition.names);
    if (!definition.tags.IsEmpty())
    {
        item[Keys::Tags] = WriteTags(definition.tags);
    }

    const auto writeIfNotDefault = [](OrderedJson& target)
    {
        return [&target](const char* key, const auto& value, const auto& defaultValue)
        {
            if (value != defaultValue)
            {
                target[key] = WriteValue(value);
            }
        };
    };

    VisitStatFields(definition, writeIfNotDefault(item));

    OrderedJson requirements = OrderedJson::object();
    VisitRequirementFields(definition.requirements, writeIfNotDefault(requirements));
    if (!requirements.empty())
    {
        item[Keys::Requirements] = std::move(requirements);
    }

    WriteByteTable(item, Keys::ClassRequirements, definition.classRequirements, ClassKeys);
    WriteByteTable(item, Keys::Resistances, definition.resistances, ResistanceKeys);
    return item;
}

// ---------------------------------------------------------------- reading

// Reads the item currently being read and collects its problems.
class ItemReader
{
public:
    ItemReader(const std::string& source, int group, std::vector<ItemDataIssue>& issues)
        : m_source(source), m_group(group), m_issues(issues)
    {
    }

    bool Read(const OrderedJson& json, ItemDefinition& definition);

private:
    void AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message);
    bool ReadIdentity(const OrderedJson& json, ItemDefinition& definition);
    bool ReadNames(const OrderedJson& json, LocalizedString& names);
    void ReadTags(const OrderedJson& json, ItemTagSet& tags);
    void ReadStats(const OrderedJson& json, ItemDefinition& definition);
    void ReadRequirements(const OrderedJson& json, ItemDefinition& definition);
    template <size_t Count>
    void ReadByteTable(const OrderedJson& json, const char* key, std::array<BYTE, Count>& values,
                       const std::array<const char*, Count>& keys);
    void WarnAboutUnknownKeys(const OrderedJson& json, const std::set<std::string, std::less<>>& knownKeys,
                              const std::string& prefix);

    template <typename T> void ReadValue(const OrderedJson& json, const std::string& field, T& value);

    const std::string& m_source;
    int m_group;
    int m_number = ItemDataIssue::NoItem;
    bool m_hasErrors = false;
    std::vector<ItemDataIssue>& m_issues;
};

void ItemReader::AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
{
    m_issues.push_back({severity, m_source, m_group, m_number, field, message});
    m_hasErrors = m_hasErrors || severity == ItemDataIssueSeverity::Error;
}

template <typename T> void ItemReader::ReadValue(const OrderedJson& json, const std::string& field, T& value)
{
    if constexpr (std::is_enum_v<T>)
    {
        if (!json.is_string() || !FindEnumValue(json.get<std::string>(), value))
        {
            AddIssue(ItemDataIssueSeverity::Error, field, "must be " + JoinEnumNames<T>());
        }
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        if (!json.is_boolean())
        {
            AddIssue(ItemDataIssueSeverity::Error, field, "must be true or false");
            return;
        }
        value = json.get<bool>();
    }
    else
    {
        constexpr long long Minimum = std::numeric_limits<T>::min();
        constexpr long long Maximum = std::numeric_limits<T>::max();
        long long number = 0;
        if (!json.is_number_integer())
        {
            AddIssue(ItemDataIssueSeverity::Error, field, "must be a whole number");
            return;
        }

        if (!ReadWholeNumber(json, number) || number < Minimum || number > Maximum)
        {
            AddIssue(ItemDataIssueSeverity::Error, field,
                     "must be between " + std::to_string(Minimum) + " and " + std::to_string(Maximum));
            return;
        }
        value = static_cast<T>(number);
    }
}

bool ItemReader::ReadIdentity(const OrderedJson& json, ItemDefinition& definition)
{
    const auto number = json.find(Keys::Number);
    long long itemNumber = 0;
    if (number == json.end() || !number->is_number_integer())
    {
        AddIssue(ItemDataIssueSeverity::Error, Keys::Number, "missing or not a whole number");
        return false;
    }

    if (!ReadWholeNumber(*number, itemNumber) || itemNumber < 0 || itemNumber >= MAX_ITEM_INDEX)
    {
        AddIssue(ItemDataIssueSeverity::Error, Keys::Number,
                 "must be between 0 and " + std::to_string(MAX_ITEM_INDEX - 1));
        return false;
    }
    m_number = static_cast<int>(itemNumber);

    const auto name = json.find(Keys::Name);
    if (name == json.end())
    {
        AddIssue(ItemDataIssueSeverity::Error, Keys::Name, "missing");
        return false;
    }
    if (!ReadNames(*name, definition.names))
    {
        return false;
    }

    definition.group = m_group;
    definition.number = m_number;
    return true;
}

bool ItemReader::ReadNames(const OrderedJson& json, LocalizedString& names)
{
    // A plain text is the English name only; handy for items written by hand.
    if (json.is_string())
    {
        names.Set(LocalizedString::NeutralLocale, json.get<std::string>());
        return true;
    }

    if (!json.is_object())
    {
        AddIssue(ItemDataIssueSeverity::Error, Keys::Name, "must be an object of \"<locale>\": \"<name>\" pairs");
        return false;
    }

    for (const auto& [locale, text] : json.items())
    {
        if (!text.is_string())
        {
            AddIssue(ItemDataIssueSeverity::Error, std::string(Keys::Name) + "." + locale, "must be a text");
            return false;
        }
        names.Set(locale, text.get<std::string>());
    }
    return true;
}

void ItemReader::ReadTags(const OrderedJson& json, ItemTagSet& tags)
{
    const auto list = json.find(Keys::Tags);
    if (list == json.end())
    {
        return;
    }
    if (!list->is_array())
    {
        AddIssue(ItemDataIssueSeverity::Error, Keys::Tags, "must be a list of tag names");
        return;
    }

    for (const OrderedJson& entry : *list)
    {
        ItemTag tag{};
        if (!entry.is_string() || !FindEnumValue(entry.get<std::string>(), tag))
        {
            AddIssue(ItemDataIssueSeverity::Error, Keys::Tags,
                     entry.dump() + " is not a tag; tags are " + JoinEnumNames<ItemTag>());
            continue;
        }
        tags.Set(tag);
    }
}

void ItemReader::ReadStats(const OrderedJson& json, ItemDefinition& definition)
{
    VisitStatFields(definition,
                    [&](const char* key, auto& value, const auto&)
                    {
                        const auto field = json.find(key);
                        if (field != json.end())
                        {
                            ReadValue(*field, key, value);
                        }
                    });
}

void ItemReader::ReadRequirements(const OrderedJson& json, ItemDefinition& definition)
{
    const auto requirements = json.find(Keys::Requirements);
    if (requirements == json.end())
    {
        return;
    }
    if (!requirements->is_object())
    {
        AddIssue(ItemDataIssueSeverity::Error, Keys::Requirements, "must be an object");
        return;
    }

    std::set<std::string, std::less<>> knownKeys;
    VisitRequirementFields(definition.requirements,
                           [&](const char* key, auto& value, const auto&)
                           {
                               knownKeys.insert(key);
                               const auto field = requirements->find(key);
                               if (field != requirements->end())
                               {
                                   ReadValue(*field, std::string(Keys::Requirements) + "." + key, value);
                               }
                           });
    WarnAboutUnknownKeys(*requirements, knownKeys, std::string(Keys::Requirements) + ".");
}

template <size_t Count>
void ItemReader::ReadByteTable(const OrderedJson& json, const char* key, std::array<BYTE, Count>& values,
                               const std::array<const char*, Count>& keys)
{
    const auto table = json.find(key);
    if (table == json.end())
    {
        return;
    }
    if (!table->is_object())
    {
        AddIssue(ItemDataIssueSeverity::Error, key, "must be an object");
        return;
    }

    std::set<std::string, std::less<>> knownKeys(keys.begin(), keys.end());
    for (size_t i = 0; i < Count; ++i)
    {
        const auto field = table->find(keys[i]);
        if (field != table->end())
        {
            ReadValue(*field, std::string(key) + "." + keys[i], values[i]);
        }
    }
    WarnAboutUnknownKeys(*table, knownKeys, std::string(key) + ".");
}

void ItemReader::WarnAboutUnknownKeys(const OrderedJson& json, const std::set<std::string, std::less<>>& knownKeys,
                                      const std::string& prefix)
{
    for (const auto& [key, value] : json.items())
    {
        if (!knownKeys.contains(key))
        {
            AddIssue(ItemDataIssueSeverity::Warning, prefix + key, "unknown field, ignored");
        }
    }
}

std::set<std::string, std::less<>> GetKnownItemKeys()
{
    std::set<std::string, std::less<>> keys{
        Keys::Number, Keys::Name, Keys::Tags, Keys::Requirements, Keys::ClassRequirements, Keys::Resistances};
    ItemDefinition unused;
    VisitStatFields(unused, [&](const char* key, auto&, const auto&) { keys.insert(key); });
    return keys;
}

bool ItemReader::Read(const OrderedJson& json, ItemDefinition& definition)
{
    if (!json.is_object())
    {
        AddIssue(ItemDataIssueSeverity::Error, "", "an item must be an object");
        return false;
    }
    if (!ReadIdentity(json, definition))
    {
        return false;
    }

    ReadTags(json, definition.tags);
    ReadStats(json, definition);
    ReadRequirements(json, definition);
    ReadByteTable(json, Keys::ClassRequirements, definition.classRequirements, ClassKeys);
    ReadByteTable(json, Keys::Resistances, definition.resistances, ResistanceKeys);

    static const std::set<std::string, std::less<>> KnownItemKeys = GetKnownItemKeys();
    WarnAboutUnknownKeys(json, KnownItemKeys, "");
    return !m_hasErrors;
}

void AddFileIssue(std::vector<ItemDataIssue>& issues, const std::string& source, int group, const std::string& field,
                  const std::string& message)
{
    issues.push_back({ItemDataIssueSeverity::Error, source, group, ItemDataIssue::NoItem, field, message});
}

bool TryParse(std::string_view text, const std::string& source, OrderedJson& root, std::vector<ItemDataIssue>& issues)
{
    try
    {
        root = OrderedJson::parse(text);
        return true;
    }
    catch (const OrderedJson::parse_error& error)
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, "", std::string("invalid JSON: ") + error.what());
        return false;
    }
}

bool ReadFormatVersion(const OrderedJson& root, const std::string& source, std::vector<ItemDataIssue>& issues)
{
    const auto version = root.find(Keys::FormatVersion);
    if (version == root.end() || !version->is_number_integer())
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::FormatVersion, "missing or not a whole number");
        return false;
    }

    long long formatVersion = 0;
    if (!ReadWholeNumber(*version, formatVersion) || formatVersion < 1 || formatVersion > ItemJsonFormatVersion)
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::FormatVersion,
                     "version " + version->dump() + " is not supported (this client reads up to " +
                         std::to_string(ItemJsonFormatVersion) + ")");
        return false;
    }
    return true;
}

bool ReadGroup(const OrderedJson& root, const std::string& source, int& group, std::vector<ItemDataIssue>& issues)
{
    const auto groupField = root.find(Keys::Group);
    if (groupField == root.end() || !groupField->is_number_integer())
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::Group, "missing or not a whole number");
        return false;
    }

    long long groupNumber = 0;
    if (!ReadWholeNumber(*groupField, groupNumber) || groupNumber < 0 || groupNumber >= MAX_ITEM_TYPE)
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::Group,
                     "must be between 0 and " + std::to_string(MAX_ITEM_TYPE - 1));
        return false;
    }
    group = static_cast<int>(groupNumber);
    return true;
}
// The JSON writer puts every list entry on its own line. Tag lists are short,
// so they are easier to read on one line: "tags": ["jewel", "valuable"].
std::string PutTagListsOnOneLine(const std::string& text)
{
    const std::string listStart = std::string("\"") + Keys::Tags + "\": [";
    std::string result;
    size_t position = 0;
    while (true)
    {
        const size_t start = text.find(listStart, position);
        const size_t end = start == std::string::npos ? std::string::npos : text.find(']', start);
        if (end == std::string::npos)
        {
            result.append(text, position, std::string::npos);
            return result;
        }

        result.append(text, position, start + listStart.size() - position);
        // Tag names have no spaces, so every newline and indentation can go;
        // only the space after each comma stays.
        for (size_t i = start + listStart.size(); i < end; ++i)
        {
            const char character = text[i];
            const bool afterComma = !result.empty() && result.back() == ',';
            if (character != '\n' && (character != ' ' || afterComma))
            {
                result += character;
            }
        }
        result += ']';
        position = end + 1;
    }
}
} // namespace

void ReadItemGroupJson(std::string_view text, const std::string& source, std::vector<ItemDefinition>& items,
                       std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!TryParse(text, source, root, issues))
    {
        return;
    }
    if (!root.is_object())
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, "", "the file must contain a JSON object");
        return;
    }

    int group = 0;
    if (!ReadFormatVersion(root, source, issues) || !ReadGroup(root, source, group, issues))
    {
        return;
    }

    const auto itemList = root.find(Keys::Items);
    if (itemList == root.end() || !itemList->is_array())
    {
        AddFileIssue(issues, source, group, Keys::Items, "missing or not a list");
        return;
    }

    for (const OrderedJson& json : *itemList)
    {
        ItemDefinition definition;
        ItemReader reader(source, group, issues);
        if (reader.Read(json, definition))
        {
            items.push_back(std::move(definition));
        }
    }
}

std::string WriteItemGroupJson(int group, std::span<const ItemDefinition> items)
{
    std::vector<const ItemDefinition*> groupItems;
    for (const ItemDefinition& definition : items)
    {
        if (definition.group == group && definition.Exists())
        {
            groupItems.push_back(&definition);
        }
    }
    std::sort(groupItems.begin(), groupItems.end(),
              [](const ItemDefinition* left, const ItemDefinition* right) { return left->number < right->number; });

    OrderedJson root;
    root[Keys::FormatVersion] = ItemJsonFormatVersion;
    root[Keys::Group] = group;
    root[Keys::Items] = OrderedJson::array();
    for (const ItemDefinition* definition : groupItems)
    {
        root[Keys::Items].push_back(WriteItem(*definition));
    }

    // Names are UTF-8 already; replace (instead of throwing on) anything that
    // is not, so a bad name can never stop a save half-way.
    return PutTagListsOnOneLine(root.dump(JsonIndent, ' ', false, OrderedJson::error_handler_t::replace)) + "\n";
}
} // namespace Data::Items
