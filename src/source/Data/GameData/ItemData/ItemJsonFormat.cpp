#include "stdafx.h"

#include "ItemJsonFormat.h"
#include "ItemEnumNames.h"
#include "ItemJsonCommon.h"
#include "ItemType.h"

#include <algorithm>
#include <array>
#include <limits>
#include <set>
#include <type_traits>

namespace Data::Items
{
namespace
{
using Json::OrderedJson;
using Json::ReadWholeNumber;

namespace Keys
{
using namespace Json::Keys;
constexpr const char* Items = "items";
constexpr const char* Name = "name";
constexpr const char* Tags = "tags";
constexpr const char* Description = "description";
constexpr const char* Text = "text";
constexpr const char* Color = "color";
constexpr const char* Levels = "levels";
constexpr const char* When = "when";
constexpr const char* Requirements = "requirements";
constexpr const char* RequirementsByLevel = "requirementsByLevel";
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
    visit("weaponKind", definition.weaponKind, WeaponKind::None);
    visit("statProfile", definition.statProfile, std::string{});
    visit("potionPriority", definition.potionPriority, BYTE{0});
    visit("twoHanded", definition.twoHanded, false);
    visit("skill", definition.skill, WORD{0});
    visit("level", definition.level, WORD{0});
    visit("durability", definition.durability, BYTE{0});
    visit("magicDurability", definition.magicDurability, BYTE{0});
    visit("damageMin", definition.damageMin, WORD{0});
    visit("damageMax", definition.damageMax, WORD{0});
    visit("blockRate", definition.blockRate, BYTE{0});
    visit("defense", definition.defense, BYTE{0});
    visit("magicDefense", definition.magicDefense, BYTE{0});
    visit("attackSpeed", definition.attackSpeed, BYTE{0});
    visit("walkSpeed", definition.walkSpeed, BYTE{0});
    visit("magicPower", definition.magicPower, WORD{0});
    visit("excellentBonus", definition.excellentBonus, BYTE{0});
    visit("excellentRaisesLevel", definition.excellentRaisesLevel, true);
    visit("attackType", definition.attackType, BYTE{0});
    visit("sellValue", definition.sellValue, BYTE{0});
    visit("buyPrice", definition.buyPrice, 0);
    visit("tradable", definition.tradable, true);
    visit("droppable", definition.droppable, true);
    visit("storable", definition.storable, true);
    visit("sellable", definition.sellable, true);
    visit("personalShopSellable", definition.personalShopSellable, true);
    visit("repairable", definition.repairable, true);
    visit("droppableWhileRented", definition.droppableWhileRented, true);
    visit("personalShopSellableWhileRented", definition.personalShopSellableWhileRented, true);
    visit("sellableWhenRentalExpired", definition.sellableWhenRentalExpired, false);
}

// Same order as ItemRequirement and VisitRequirementFields.
constexpr std::array<const char*, ItemRequirementCount> RequirementKeys = {"level",  "strength", "dexterity",
                                                                           "energy", "vitality", "leadership"};

template <typename TRequirements, typename TVisitor>
void VisitRequirementFields(TRequirements& requirements, TVisitor&& visit)
{
    const auto key = [](ItemRequirement requirement) { return RequirementKeys[static_cast<size_t>(requirement)]; };
    visit(key(ItemRequirement::Level), requirements.level, WORD{0});
    visit(key(ItemRequirement::Strength), requirements.strength, WORD{0});
    visit(key(ItemRequirement::Dexterity), requirements.dexterity, WORD{0});
    visit(key(ItemRequirement::Energy), requirements.energy, WORD{0});
    visit(key(ItemRequirement::Vitality), requirements.vitality, WORD{0});
    visit(key(ItemRequirement::Leadership), requirements.leadership, WORD{0});
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

// One object per line: its text (as a name is written, "@name" for a shared text, "" for a blank line), its conditions
// unless it has none, its color unless white, and its item levels unless it is shown at every level.
OrderedJson WriteDescription(const std::vector<ItemDescriptionLine>& description)
{
    OrderedJson json = OrderedJson::array();
    for (const ItemDescriptionLine& line : description)
    {
        OrderedJson entry;
        // A blank line is a plain empty text, as it is written by hand.
        entry[Keys::Text] = !line.textName.empty() ? OrderedJson("@" + line.textName)
                            : line.text.IsEmpty()  ? OrderedJson("")
                                                   : Json::WriteLocalizedString(line.text);
        if (!line.when.empty())
        {
            OrderedJson when = OrderedJson::array();
            for (const TooltipCondition& condition : line.when)
            {
                when.push_back(WriteTooltipCondition(condition));
            }
            entry[Keys::When] = std::move(when);
        }
        if (line.color != ItemTextColor::White)
        {
            entry[Keys::Color] = WriteValue(line.color);
        }
        if (line.HasLevels())
        {
            entry[Keys::Levels] = OrderedJson::array({line.minLevel, line.maxLevel});
        }
        json.push_back(std::move(entry));
    }
    return json;
}

OrderedJson WriteItemObject(const ItemDefinition& definition)
{
    OrderedJson item;
    item[Keys::Number] = definition.number;
    item[Keys::Name] = Json::WriteLocalizedString(definition.names);
    if (!definition.tags.IsEmpty())
    {
        item[Keys::Tags] = WriteTags(definition.tags);
    }
    if (!definition.description.empty())
    {
        item[Keys::Description] = WriteDescription(definition.description);
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

    OrderedJson requirementsByLevel = OrderedJson::object();
    for (size_t requirement = 0; requirement < ItemRequirementCount; ++requirement)
    {
        const RequirementByLevel* values =
            definition.requirementsByLevel.Find(static_cast<ItemRequirement>(requirement));
        if (values == nullptr)
        {
            continue;
        }
        requirementsByLevel[RequirementKeys[requirement]] = Json::WritePerLevelValues(*values);
    }
    if (!requirementsByLevel.empty())
    {
        item[Keys::RequirementsByLevel] = std::move(requirementsByLevel);
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
    void ReadDescription(const OrderedJson& json, std::vector<ItemDescriptionLine>& description);
    void ReadDescriptionLevels(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line);
    void ReadDescriptionConditions(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line);
    void ReadStats(const OrderedJson& json, ItemDefinition& definition);
    void ReadRequirements(const OrderedJson& json, ItemDefinition& definition);
    void ReadRequirementsByLevel(const OrderedJson& json, ItemDefinition& definition);
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
        // Files saved before phase 3 have the slot as a number. A number that
        // has a name is still read, so those files load; saving writes the name.
        long long number = 0;
        if (json.is_number_integer() && ReadWholeNumber(json, number) && number >= 0 &&
            number <= std::numeric_limits<std::underlying_type_t<T>>::max() &&
            FindEnumName(static_cast<T>(number)) != nullptr)
        {
            value = static_cast<T>(number);
            AddIssue(ItemDataIssueSeverity::Warning, field,
                     std::string("is a number; saving writes it as \"") + FindEnumName(value) + "\"");
        }
        else if (!json.is_string() || !FindEnumValue(json.get<std::string>(), value))
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
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (!json.is_string() || !Json::IsName(json.get_ref<const std::string&>()))
        {
            AddIssue(ItemDataIssueSeverity::Error, field, "must be a name of letters and digits");
            return;
        }
        value = json.get<std::string>();
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
    const auto addError = [this](const std::string& field, const std::string& message)
    { AddIssue(ItemDataIssueSeverity::Error, field, message); };
    if (!Json::ReadNumber(json, m_number, addError))
    {
        return false;
    }

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
    return Json::ReadLocalizedString(json, Keys::Name, names,
                                     [this](const std::string& field, const std::string& message)
                                     { AddIssue(ItemDataIssueSeverity::Error, field, message); });
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

void ItemReader::ReadDescription(const OrderedJson& json, std::vector<ItemDescriptionLine>& description)
{
    const auto list = json.find(Keys::Description);
    if (list == json.end())
    {
        return;
    }
    if (!list->is_array())
    {
        AddIssue(ItemDataIssueSeverity::Error, Keys::Description, "must be a list of lines");
        return;
    }

    for (size_t index = 0; index < list->size(); ++index)
    {
        const OrderedJson& entry = (*list)[index];
        const std::string field = std::string(Keys::Description) + "[" + std::to_string(index) + "]";
        const auto text = entry.is_object() ? entry.find(Keys::Text) : entry.end();
        if (!entry.is_object() || text == entry.end())
        {
            AddIssue(ItemDataIssueSeverity::Error, field, "must be an object with a text");
            continue;
        }

        ItemDescriptionLine line;
        bool read = true;
        if (text->is_string() && text->get<std::string>().starts_with("@"))
        {
            // A shared text of the tooltip file; ValidateDescriptionTexts checks that it has it.
            line.textName = text->get<std::string>().substr(1);
        }
        else
        {
            read = Json::ReadLocalizedString(*text, field + "." + Keys::Text, line.text,
                                             [this](const std::string& errorField, const std::string& message)
                                             { AddIssue(ItemDataIssueSeverity::Error, errorField, message); });
        }
        if (const auto when = entry.find(Keys::When); when != entry.end())
        {
            ReadDescriptionConditions(*when, field + "." + Keys::When, line);
        }
        const auto color = entry.find(Keys::Color);
        if (color != entry.end())
        {
            ReadValue(*color, field + "." + Keys::Color, line.color);
        }
        const auto levels = entry.find(Keys::Levels);
        if (levels != entry.end())
        {
            ReadDescriptionLevels(*levels, field + "." + Keys::Levels, line);
        }
        WarnAboutUnknownKeys(entry, {Keys::Text, Keys::When, Keys::Color, Keys::Levels}, field + ".");
        if (read)
        {
            description.push_back(std::move(line));
        }
    }
}

// The conditions a description line is shown on, all of them: a list of texts such as "durability" or "!timed".
void ItemReader::ReadDescriptionConditions(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line)
{
    if (!json.is_array())
    {
        AddIssue(ItemDataIssueSeverity::Error, field,
                 "must be a list of conditions, such as [\"durability\", \"!timed\"]");
        return;
    }
    for (size_t index = 0; index < json.size(); ++index)
    {
        const std::string entryField = field + "[" + std::to_string(index) + "]";
        TooltipCondition condition;
        std::string error;
        if (!json[index].is_string())
        {
            AddIssue(ItemDataIssueSeverity::Error, entryField, "must be a text");
        }
        else if (!ReadTooltipCondition(json[index].get<std::string>(), condition, error))
        {
            AddIssue(ItemDataIssueSeverity::Error, entryField, error);
        }
        else
        {
            line.when.push_back(std::move(condition));
        }
    }
}

// [first, last]: the item levels a description line is shown at, both included.
void ItemReader::ReadDescriptionLevels(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line)
{
    // Read as a whole number first: a number outside int's range would wrap into a level.
    const auto readLevel = [](const OrderedJson& value, long long& level)
    { return ReadWholeNumber(value, level) && level >= 0 && level < ItemLevelCount; };
    long long first = 0;
    long long last = 0;
    if (!json.is_array() || json.size() != 2 || !readLevel(json[0], first) || !readLevel(json[1], last) || first > last)
    {
        AddIssue(ItemDataIssueSeverity::Error, field,
                 "must be [first, last], two item levels from 0 to " + std::to_string(ItemLevelCount - 1) +
                     " with the first not above the last");
        return;
    }
    line.minLevel = static_cast<int>(first);
    line.maxLevel = static_cast<int>(last);
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
    Json::WarnAboutUnknownKeys(json, knownKeys, prefix, [this](const std::string& field, const std::string& message)
                               { AddIssue(ItemDataIssueSeverity::Warning, field, message); });
}

void ItemReader::ReadRequirementsByLevel(const OrderedJson& json, ItemDefinition& definition)
{
    const auto byLevel = json.find(Keys::RequirementsByLevel);
    if (byLevel == json.end())
    {
        return;
    }
    if (!byLevel->is_object())
    {
        AddIssue(ItemDataIssueSeverity::Error, Keys::RequirementsByLevel, "must be an object");
        return;
    }
    for (const auto& [key, values] : byLevel->items())
    {
        const std::string field = std::string(Keys::RequirementsByLevel) + "." + key;
        const auto requirement = std::find(RequirementKeys.begin(), RequirementKeys.end(), key);
        if (requirement == RequirementKeys.end())
        {
            AddIssue(ItemDataIssueSeverity::Error, field, "is not a requirement");
            continue;
        }
        RequirementByLevel levels{};
        const bool read = Json::ReadPerLevelValues(
            values, field, levels, [this](const OrderedJson& entry, const std::string& entryField, WORD& value)
            { ReadValue(entry, entryField, value); }, [this](const std::string& errorField, const std::string& message)
            { AddIssue(ItemDataIssueSeverity::Error, errorField, message); });
        if (read)
        {
            definition.requirementsByLevel.Set(static_cast<ItemRequirement>(requirement - RequirementKeys.begin()),
                                               levels);
        }
    }
}

std::set<std::string, std::less<>> GetKnownItemKeys()
{
    std::set<std::string, std::less<>> keys{Keys::Number,
                                            Keys::Name,
                                            Keys::Tags,
                                            Keys::Description,
                                            Keys::Requirements,
                                            Keys::RequirementsByLevel,
                                            Keys::ClassRequirements,
                                            Keys::Resistances};
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
    ReadDescription(json, definition.description);
    ReadStats(json, definition);
    ReadRequirements(json, definition);
    ReadRequirementsByLevel(json, definition);
    ReadByteTable(json, Keys::ClassRequirements, definition.classRequirements, ClassKeys);
    ReadByteTable(json, Keys::Resistances, definition.resistances, ResistanceKeys);

    static const std::set<std::string, std::less<>> KnownItemKeys = GetKnownItemKeys();
    WarnAboutUnknownKeys(json, KnownItemKeys, "");
    return !m_hasErrors;
}

} // namespace

void ReadItemGroupJson(std::string_view text, const std::string& source, std::vector<ItemDefinition>& items,
                       std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    int group = 0;
    if (!Json::ReadFileHeader(text, source, ItemJsonFormatVersion, root, group, issues))
    {
        return;
    }

    const auto itemList = root.find(Keys::Items);
    if (itemList == root.end() || !itemList->is_array())
    {
        Json::AddFileIssue(issues, source, group, Keys::Items, "missing or not a list");
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
        root[Keys::Items].push_back(WriteItemObject(*definition));
    }

    // Names are UTF-8 already; replace (instead of throwing on) anything that
    // is not, so a bad name can never stop a save half-way.
    return LayOutItemJsonText(root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace)) + "\n";
}

OrderedJson WriteItemJson(const ItemDefinition& definition)
{
    return WriteItemObject(definition);
}

bool ReadItemJson(const OrderedJson& json, const std::string& source, int group, ItemDefinition& definition,
                  std::vector<ItemDataIssue>& issues)
{
    ItemReader reader(source, group, issues);
    return reader.Read(json, definition);
}

std::string LayOutItemJsonText(const std::string& text)
{
    std::string laidOut = Json::PutListsOnOneLine(text, Keys::Tags);
    laidOut = Json::PutListsOnOneLine(laidOut, Keys::Levels);
    laidOut = Json::PutListsOnOneLine(laidOut, Keys::When);
    return Json::PutObjectsOnOneLine(laidOut, Keys::RequirementsByLevel);
}
} // namespace Data::Items
