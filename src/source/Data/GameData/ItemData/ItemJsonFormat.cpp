#include "stdafx.h"

#include "ItemJsonFormat.h"
#include "ItemDescriptionPlaceholders.h"
#include "ItemEnumNames.h"
#include "ItemJsonCommon.h"
#include "ItemTooltipConditionsJson.h"
#include "ItemTooltipLayouts.h"
#include "ItemType.h"
#include "Data/Translation/ItemReference.h"

#include <algorithm>
#include <array>
#include <limits>
#include <set>
#include <string_view>
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
constexpr const char* Options = "options";
constexpr const char* Description = "description";
constexpr const char* Text = "text";
constexpr const char* Color = "color";
constexpr const char* Levels = "levels";
constexpr const char* When = "when";
constexpr const char* Values = "values";
constexpr const char* LevelValues = "levelValues";
constexpr const char* NameLines = "nameLines";
constexpr const char* NameForms = "nameForms";
constexpr const char* GroundLabels = "groundLabels";
constexpr const char* TooltipLayout = "tooltipLayout";
constexpr const char* TooltipLayoutChanges = "tooltipLayoutChanges";
constexpr const char* Hide = "hide";
constexpr const char* Add = "add";
constexpr const char* Lines = "lines";
constexpr const char* Style = "style";
constexpr const char* Colors = "colors";
constexpr const char* Bold = "bold";
constexpr const char* After = "after";
constexpr const char* AfterOptions = "options";
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
// One line of a description or of the name lines: its text (as a name is written, "@name" for a shared text, "" for a
// blank line) when `writeText`, its values, conditions, color when `writeColor`, color rules, bold, levels unless every
// level, and its place.
OrderedJson WriteLine(const ItemDescriptionLine& line, bool writeText, bool writeColor)
{
    OrderedJson entry;
    // A blank line is a plain empty text, as it is written by hand.
    if (writeText)
    {
        entry[Keys::Text] = !line.textName.empty() ? OrderedJson("@" + line.textName)
                            : line.text.IsEmpty()  ? OrderedJson("")
                                                   : Json::WriteLocalizedString(line.text);
    }
    if (!line.values.empty())
    {
        OrderedJson values = OrderedJson::object();
        for (const auto& [placeholder, value] : line.values)
        {
            values[placeholder] = value;
        }
        entry[Keys::Values] = std::move(values);
    }
    if (!line.when.empty())
    {
        OrderedJson when = OrderedJson::array();
        for (const TooltipCondition& condition : line.when)
        {
            when.push_back(WriteTooltipCondition(condition));
        }
        entry[Keys::When] = std::move(when);
    }
    if (writeColor)
    {
        entry[Keys::Color] = WriteValue(line.color);
    }
    if (!line.colors.empty())
    {
        OrderedJson colors = OrderedJson::array();
        for (const TooltipColorRule& rule : line.colors)
        {
            OrderedJson when = OrderedJson::array();
            for (const TooltipCondition& condition : rule.when)
            {
                when.push_back(WriteTooltipCondition(condition));
            }
            colors.push_back(OrderedJson{{Keys::When, std::move(when)}, {Keys::Color, WriteValue(rule.color)}});
        }
        entry[Keys::Colors] = std::move(colors);
    }
    if (line.bold)
    {
        entry[Keys::Bold] = true;
    }
    if (line.HasLevels())
    {
        entry[Keys::Levels] = OrderedJson::array({line.minLevel, line.maxLevel});
    }
    if (line.afterOptions)
    {
        entry[Keys::After] = Keys::AfterOptions;
    }
    return entry;
}

OrderedJson WriteDescription(const std::vector<ItemDescriptionLine>& description)
{
    OrderedJson json = OrderedJson::array();
    for (const ItemDescriptionLine& line : description)
    {
        OrderedJson entry = WriteLine(line, true, line.color != ItemTextColor::White);
        json.push_back(std::move(entry));
    }
    return json;
}

// {"pl": {"gen": "<genitive>", "acc": "<accusative>"}}: the languages sorted by code, each one's forms in the order of
// the case keys.
OrderedJson WriteNameForms(const ItemNameForms& forms)
{
    OrderedJson json = OrderedJson::object();
    for (const auto& [locale, caseForms] : forms.GetAll())
    {
        OrderedJson entry = OrderedJson::object();
        for (const std::string_view caseKey : Data::Translation::ItemCaseKeys)
        {
            if (const auto form = caseForms.find(caseKey); form != caseForms.end())
            {
                entry[std::string(caseKey)] = form->second;
            }
        }
        json[locale] = std::move(entry);
    }
    return json;
}

// A name line's fields with its forms right after its text.
OrderedJson WriteNameLine(const ItemNameLine& line)
{
    OrderedJson entry = WriteLine(line, line.hasText, line.hasColor);
    if (line.nameForms.IsEmpty())
    {
        return entry;
    }
    OrderedJson ordered = OrderedJson::object();
    if (const auto text = entry.find(Keys::Text); text != entry.end())
    {
        ordered[Keys::Text] = *text;
    }
    ordered[Keys::NameForms] = WriteNameForms(line.nameForms);
    for (const auto& [key, value] : entry.items())
    {
        if (key != Keys::Text)
        {
            ordered[key] = value;
        }
    }
    return ordered;
}

// The name lines: a description line's fields, the text and the color only when the line has them.
OrderedJson WriteNameLines(const std::vector<ItemNameLine>& nameLines)
{
    OrderedJson json = OrderedJson::array();
    for (const ItemNameLine& line : nameLines)
    {
        json.push_back(WriteNameLine(line));
    }
    return json;
}

// {"stat": ["ENG", {"en": "STA", "de": "VIT"}, "@name", ...]}: each value as a name is written, or "@name".
OrderedJson WriteLevelValues(const std::vector<ItemLevelValues>& levelValues)
{
    OrderedJson json = OrderedJson::object();
    for (const ItemLevelValues& entry : levelValues)
    {
        OrderedJson values = OrderedJson::array();
        for (const ItemLevelValue& value : entry.values)
        {
            values.push_back(!value.textName.empty() ? OrderedJson("@" + value.textName)
                                                     : Json::WriteLocalizedString(value.text));
        }
        json[entry.name] = std::move(values);
    }
    return json;
}

// [{"style": "ticket"}, {"style": "giftBoxPink", "levels": [0, 1]}]: each rule's style, and its levels unless it holds
// at every level.
OrderedJson WriteGroundLabels(const std::vector<ItemGroundLabel>& groundLabels)
{
    OrderedJson json = OrderedJson::array();
    for (const ItemGroundLabel& label : groundLabels)
    {
        OrderedJson entry;
        entry[Keys::Style] = label.style;
        if (label.hasLevels)
        {
            entry[Keys::Levels] = OrderedJson::array({label.minLevel, label.maxLevel});
        }
        json.push_back(std::move(entry));
    }
    return json;
}

// {"hide": ["blankAfterOptions"], "add": [{"after": "classLines", "lines": [{"text": ""}]}]}: the ids of the layout
// lines the item leaves out, and the lines it adds, each written as a description line.
OrderedJson WriteTooltipLayoutChanges(const ItemTooltipLayoutChanges& changes)
{
    OrderedJson json = OrderedJson::object();
    if (!changes.hide.empty())
    {
        json[Keys::Hide] = changes.hide;
    }
    if (!changes.add.empty())
    {
        OrderedJson add = OrderedJson::array();
        for (const ItemTooltipLayoutAddition& addition : changes.add)
        {
            add.push_back(OrderedJson{{Keys::After, addition.after}, {Keys::Lines, WriteDescription(addition.lines)}});
        }
        json[Keys::Add] = std::move(add);
    }
    return json;
}

OrderedJson WriteItemObject(const ItemDefinition& definition)
{
    OrderedJson item;
    item[Keys::Number] = definition.number;
    // An entry without a name has none.
    if (!definition.names.IsEmpty())
    {
        item[Keys::Name] = Json::WriteLocalizedString(definition.names);
    }
    if (!definition.nameForms.IsEmpty())
    {
        item[Keys::NameForms] = WriteNameForms(definition.nameForms);
    }
    if (!definition.tags.IsEmpty())
    {
        item[Keys::Tags] = WriteTags(definition.tags);
    }
    if (!definition.options.empty())
    {
        item[Keys::Options] = definition.options;
    }
    if (!definition.description.empty())
    {
        item[Keys::Description] = WriteDescription(definition.description);
    }
    if (!definition.levelValues.empty())
    {
        item[Keys::LevelValues] = WriteLevelValues(definition.levelValues);
    }
    if (!definition.nameLines.empty())
    {
        item[Keys::NameLines] = WriteNameLines(definition.nameLines);
    }
    if (!definition.groundLabels.empty())
    {
        item[Keys::GroundLabels] = WriteGroundLabels(definition.groundLabels);
    }
    if (!definition.tooltipLayout.empty())
    {
        item[Keys::TooltipLayout] = definition.tooltipLayout;
    }
    if (!definition.tooltipLayoutChanges.IsEmpty())
    {
        item[Keys::TooltipLayoutChanges] = WriteTooltipLayoutChanges(definition.tooltipLayoutChanges);
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
    ItemReader(const std::string& source, int group, std::vector<ItemDataIssue>& issues, bool nameRequired = true)
        : m_source(source), m_group(group), m_issues(issues), m_nameRequired(nameRequired)
    {
    }

    bool Read(const OrderedJson& json, ItemDefinition& definition);

private:
    void AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message);
    bool ReadIdentity(const OrderedJson& json, ItemDefinition& definition);
    bool ReadNames(const OrderedJson& json, LocalizedString& names);
    void ReadNameForms(const OrderedJson& json, const std::string& field, ItemNameForms& forms);
    bool ReadCaseForms(const OrderedJson& json, const std::string& locale, const std::string& field,
                       ItemNameForms& forms);
    void ReadTags(const OrderedJson& json, ItemTagSet& tags);
    void ReadOptionLinks(const OrderedJson& json, std::vector<std::string>& options);
    void ReadDescription(const OrderedJson& json, std::vector<ItemDescriptionLine>& description);
    void ReadNameLines(const OrderedJson& json, std::vector<ItemNameLine>& nameLines);
    void ReadGroundLabels(const OrderedJson& json, std::vector<ItemGroundLabel>& groundLabels);
    void ReadTooltipLayoutChanges(const OrderedJson& json, ItemTooltipLayoutChanges& changes);
    void ReadTooltipLayoutAddition(const OrderedJson& entry, const std::string& field,
                                   ItemTooltipLayoutChanges& changes);
    void ApplyRemovedTooltipTags(ItemDefinition& definition);
    bool ReadLevelRange(const OrderedJson& json, const std::string& field, int& first, int& last);
    bool ReadLine(const OrderedJson& entry, const std::string& field, size_t maxLength, ItemDescriptionLine& line);
    void ReadDescriptionLevels(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line);
    void ReadDescriptionConditions(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line);
    void ReadDescriptionValues(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line);
    void ReadDescriptionColors(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line);
    bool ReadConditionList(const OrderedJson& json, const std::string& field, std::vector<TooltipCondition>& when);
    void ReadLevelValues(const OrderedJson& json, std::vector<ItemLevelValues>& levelValues);
    void ReadStats(const OrderedJson& json, ItemDefinition& definition);
    void ReadRequirements(const OrderedJson& json, ItemDefinition& definition);
    void ReadRequirementsByLevel(const OrderedJson& json, ItemDefinition& definition);
    template <size_t Count>
    void ReadByteTable(const OrderedJson& json, const char* key, std::array<BYTE, Count>& values,
                       const std::array<const char*, Count>& keys);
    void WarnAboutUnknownKeys(const OrderedJson& json, const std::set<std::string, std::less<>>& knownKeys,
                              const std::string& prefix);
    void WarnAboutLongText(const LocalizedString& text, const std::string& field, size_t maxLength);

    template <typename T> void ReadValue(const OrderedJson& json, const std::string& field, T& value);

    const std::string& m_source;
    int m_group;
    int m_number = ItemDataIssue::NoItem;
    bool m_hasFatal = false;
    std::vector<ItemDataIssue>& m_issues;
    // The tags of the item that the tooltip layout presets replaced, read as their replacements.
    std::vector<const RemovedTooltipTag*> m_removedTags;
    // An item file's entry without a name has none, and so has an item of an exchange file
    // that does not carry the names.
    bool m_nameRequired = true;
};

void ItemReader::AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
{
    m_issues.push_back({severity, m_source, m_group, m_number, field, message});
    m_hasFatal = m_hasFatal || severity == ItemDataIssueSeverity::Fatal;
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
            AddIssue(ItemDataIssueSeverity::Fatal, field, "must be " + JoinEnumNames<T>());
        }
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        if (!json.is_boolean())
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field, "must be true or false");
            return;
        }
        value = json.get<bool>();
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (!json.is_string() || !Json::IsName(json.get_ref<const std::string&>()))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field, "must be a name of letters and digits");
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
            AddIssue(ItemDataIssueSeverity::Fatal, field, "must be a whole number");
            return;
        }

        if (!ReadWholeNumber(json, number) || number < Minimum || number > Maximum)
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field,
                     "must be between " + std::to_string(Minimum) + " and " + std::to_string(Maximum));
            return;
        }
        value = static_cast<T>(number);
    }
}

bool ItemReader::ReadIdentity(const OrderedJson& json, ItemDefinition& definition)
{
    const auto addError = [this](const std::string& field, const std::string& message)
    { AddIssue(ItemDataIssueSeverity::Fatal, field, message); };
    if (!Json::ReadNumber(json, m_number, addError))
    {
        return false;
    }

    const auto name = json.find(Keys::Name);
    if (name == json.end())
    {
        if (m_nameRequired)
        {
            AddIssue(ItemDataIssueSeverity::Fatal, Keys::Name, "missing");
            return false;
        }
    }
    else if (!ReadNames(*name, definition.names))
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
                                     { AddIssue(ItemDataIssueSeverity::Fatal, field, message); });
}

// {"pl": {"gen": "<genitive>", "acc": "<accusative>"}}: by language code, each language's forms by case key. Whether a
// language declines names and has a name to decline is checked with the item data (ValidateNameForms).
void ItemReader::ReadNameForms(const OrderedJson& json, const std::string& field, ItemNameForms& forms)
{
    if (!json.is_object())
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field,
                 R"(must be an object of languages, each with its forms by case: {"pl": {"gen": "..."}})");
        return;
    }
    for (const auto& [locale, caseForms] : json.items())
    {
        const std::string localeField = field + "." + locale;
        if (!LocalizedString::IsValidLocaleCode(locale))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, localeField,
                     "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
            continue;
        }
        if (!ReadCaseForms(caseForms, locale, localeField, forms))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, localeField,
                     R"(must be an object of forms by case: {"gen": "..."})");
        }
    }
}

// One language's forms; false when it is not an object.
bool ItemReader::ReadCaseForms(const OrderedJson& json, const std::string& locale, const std::string& field,
                               ItemNameForms& forms)
{
    if (!json.is_object())
    {
        return false;
    }
    for (const auto& [caseKey, form] : json.items())
    {
        const std::string caseField = field + "." + caseKey;
        if (!Data::Translation::IsItemCaseKey(caseKey))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, caseField, "is not a case: gen, dat, acc, ins or loc");
        }
        else if (!form.is_string() || form.get<std::string>().empty())
        {
            AddIssue(ItemDataIssueSeverity::Fatal, caseField, "must be the name's form, a text");
        }
        else
        {
            forms.Set(locale, caseKey, form.get<std::string>());
        }
    }
    return true;
}

// The presets and line ids of the shipped Tooltip.json that replace the tags (a test checks that the file has them).
constexpr std::string_view LevelRequirementLineIds[] = {"requiredLevel", "lackingLevel"};
constexpr std::string_view BlankAfterOptionsLineIds[] = {"blankAfterOptions"};
constexpr RemovedTooltipTag RemovedTooltipTags[] = {
    {"noClassLines", "noClassLines", "noClassOrDurabilityLines", {}},
    {"noDurabilityLine", "noDurabilityLine", "noClassOrDurabilityLines", {}},
    {"noLevelRequirement", {}, {}, LevelRequirementLineIds},
    {"noBlankAfterOptions", {}, {}, BlankAfterOptionsLineIds},
};

// The removed tag a tags entry names; nullptr for another entry.
const RemovedTooltipTag* FindRemovedTagEntry(const OrderedJson& entry)
{
    return entry.is_string() ? FindRemovedTooltipTag(entry.get_ref<const std::string&>()) : nullptr;
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
        AddIssue(ItemDataIssueSeverity::Fatal, Keys::Tags, "must be a list of tag names");
        return;
    }

    for (const OrderedJson& entry : *list)
    {
        if (const RemovedTooltipTag* removed = FindRemovedTagEntry(entry))
        {
            // A file from before the presets: the tag is read as what replaces it (ApplyRemovedTooltipTags).
            m_removedTags.push_back(removed);
            AddIssue(ItemDataIssueSeverity::Warning, Keys::Tags,
                     entry.dump() + " is no tag any more: it is read as " + DescribeRemovedTooltipTag(*removed) +
                         ", which saving writes instead (docs/item-data.md, Presets)");
            continue;
        }
        ItemTag tag{};
        if (!entry.is_string() || !FindEnumValue(entry.get<std::string>(), tag))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, Keys::Tags,
                     entry.dump() + " is not a tag; tags are " + JoinEnumNames<ItemTag>());
            continue;
        }
        tags.Set(tag);
    }
}

// The option definitions the item links, by name; whether they exist is checked with them (ValidateItemOptionLinks).
void ItemReader::ReadOptionLinks(const OrderedJson& json, std::vector<std::string>& options)
{
    const auto list = json.find(Keys::Options);
    if (list == json.end())
    {
        return;
    }
    const bool names =
        list->is_array() && std::all_of(list->begin(), list->end(), [](const OrderedJson& entry)
                                        { return entry.is_string() && Json::IsName(entry.get<std::string>()); });
    if (!names)
    {
        AddIssue(ItemDataIssueSeverity::Fatal, Keys::Options, "must be a list of option definition names");
        return;
    }
    for (const OrderedJson& entry : *list)
    {
        std::string name = entry.get<std::string>();
        if (std::find(options.begin(), options.end(), name) != options.end())
        {
            AddIssue(ItemDataIssueSeverity::Warning, Keys::Options,
                     "\"" + name + "\" is in the list more than once; it is read once");
            continue;
        }
        options.push_back(std::move(name));
    }
}

// The removed tags the item file still has, as what replaces them (D73): the preset they leave the lines out with (for
// both noClassLines and noDurabilityLine the one that leaves out both), and the line ids they hide among the item's
// own changes. An item that names another layout itself keeps it, and the tag is an error: which of the two layouts
// it means is the author's call.
void ItemReader::ApplyRemovedTooltipTags(ItemDefinition& definition)
{
    std::string_view preset;
    std::vector<std::string>& hide = definition.tooltipLayoutChanges.hide;
    for (const RemovedTooltipTag* removed : m_removedTags)
    {
        for (const std::string_view id : removed->hide)
        {
            if (std::find(hide.begin(), hide.end(), id) == hide.end())
            {
                hide.emplace_back(id);
            }
        }
        if (!removed->preset.empty())
        {
            preset = preset.empty() || preset == removed->preset ? removed->preset : removed->bothPreset;
        }
    }
    if (preset.empty() || definition.tooltipLayout == preset)
    {
        return;
    }
    if (!definition.tooltipLayout.empty())
    {
        AddIssue(ItemDataIssueSeverity::Fatal, Keys::Tags,
                 "the removed tags are read as the layout \"" + std::string(preset) + "\", but the item names \"" +
                     definition.tooltipLayout + "\": write the layout it uses without the tags");
        return;
    }
    definition.tooltipLayout = preset;
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
        AddIssue(ItemDataIssueSeverity::Fatal, Keys::Description, "must be a list of lines");
        return;
    }
    for (size_t index = 0; index < list->size(); ++index)
    {
        const OrderedJson& entry = (*list)[index];
        const std::string field = std::string(Keys::Description) + "[" + std::to_string(index) + "]";
        if (!entry.is_object() || !entry.contains(Keys::Text))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field, "must be an object with a text");
            continue;
        }
        ItemDescriptionLine line;
        if (ReadLine(entry, field, MaxDescriptionLineLength, line))
        {
            description.push_back(std::move(line));
        }
    }
}

// The name lines, each a description line whose text and color may be left out.
void ItemReader::ReadNameLines(const OrderedJson& json, std::vector<ItemNameLine>& nameLines)
{
    const auto list = json.find(Keys::NameLines);
    if (list == json.end())
    {
        return;
    }
    if (!list->is_array())
    {
        AddIssue(ItemDataIssueSeverity::Fatal, Keys::NameLines, "must be a list of name lines");
        return;
    }
    for (size_t index = 0; index < list->size(); ++index)
    {
        const OrderedJson& entry = (*list)[index];
        const std::string field = std::string(Keys::NameLines) + "[" + std::to_string(index) + "]";
        if (!entry.is_object())
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field, "must be an object");
            continue;
        }
        // The name is always bold and always the first row.
        for (const char* key : {Keys::Bold, Keys::After})
        {
            if (entry.contains(key))
            {
                AddIssue(ItemDataIssueSeverity::Fatal, field + "." + key, "does not apply to a name line");
            }
        }
        ItemNameLine line;
        line.hasText = entry.contains(Keys::Text);
        line.hasColor = entry.contains(Keys::Color);
        // The line's fields without its forms, which ReadLine doesn't know; copied only for a line that has them.
        const OrderedJson* lineFields = &entry;
        OrderedJson withoutForms;
        if (const auto forms = entry.find(Keys::NameForms); forms != entry.end())
        {
            ReadNameForms(*forms, field + "." + Keys::NameForms, line.nameForms);
            withoutForms = entry;
            withoutForms.erase(Keys::NameForms);
            lineFields = &withoutForms;
        }
        if (ReadLine(*lineFields, field, MaxNameLineLength, line))
        {
            nameLines.push_back(std::move(line));
        }
    }
}

// The ground label rules, each {"style": "<name>"} with "levels" when it holds only at those levels. Whether the style
// is in the ground label file is checked with it (ValidateGroundLabels).
void ItemReader::ReadGroundLabels(const OrderedJson& json, std::vector<ItemGroundLabel>& groundLabels)
{
    const auto list = json.find(Keys::GroundLabels);
    if (list == json.end())
    {
        return;
    }
    if (!list->is_array())
    {
        AddIssue(ItemDataIssueSeverity::Fatal, Keys::GroundLabels,
                 R"(must be a list of rules {"style": "<name>"}, with "levels": [first, last] when needed)");
        return;
    }
    for (size_t index = 0; index < list->size(); ++index)
    {
        const OrderedJson& entry = (*list)[index];
        const std::string field = std::string(Keys::GroundLabels) + "[" + std::to_string(index) + "]";
        const auto style = entry.is_object() ? entry.find(Keys::Style) : entry.end();
        if (style == entry.end() || !style->is_string() || !Json::IsName(style->get<std::string>()))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field,
                     R"(must be {"style": "<name>"}, the name of a style of the ground label file)");
            continue;
        }
        ItemGroundLabel label;
        label.style = style->get<std::string>();
        if (const auto levels = entry.find(Keys::Levels); levels != entry.end())
        {
            label.hasLevels = ReadLevelRange(*levels, field + "." + Keys::Levels, label.minLevel, label.maxLevel);
        }
        WarnAboutUnknownKeys(entry, {Keys::Style, Keys::Levels}, field + ".");
        groundLabels.push_back(std::move(label));
    }
}

// The item's own changes to its tooltip layout: {"hide": ["<line id>", ...], "add": [{"after": "<line id>", "lines":
// [...]}]}, the added lines written as description lines without "after". Whether its layout has those ids is checked
// with the tooltip file (CheckItemTooltipLayout).
void ItemReader::ReadTooltipLayoutChanges(const OrderedJson& json, ItemTooltipLayoutChanges& changes)
{
    const auto found = json.find(Keys::TooltipLayoutChanges);
    if (found == json.end())
    {
        return;
    }
    const std::string field = Keys::TooltipLayoutChanges;
    if (!found->is_object())
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field,
                 R"(must be {"hide": ["<line id>", ...], "add": [{"after": "<line id>", "lines": [...]}]})");
        return;
    }
    if (const auto hide = found->find(Keys::Hide); hide != found->end() && !Json::ReadLineIds(*hide, changes.hide))
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field + "." + Keys::Hide, Json::LineIdsMessage);
    }
    if (const auto add = found->find(Keys::Add); add != found->end())
    {
        if (!add->is_array())
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field + "." + Keys::Add, Json::LineAdditionsMessage);
        }
        else
        {
            for (size_t index = 0; index < add->size(); ++index)
            {
                ReadTooltipLayoutAddition((*add)[index], field + "." + Keys::Add + "[" + std::to_string(index) + "]",
                                          changes);
            }
        }
    }
    WarnAboutUnknownKeys(*found, {Keys::Hide, Keys::Add}, field + ".");
}

void ItemReader::ReadTooltipLayoutAddition(const OrderedJson& entry, const std::string& field,
                                           ItemTooltipLayoutChanges& changes)
{
    ItemTooltipLayoutAddition addition;
    const OrderedJson* lines = Json::ReadLineAddition(entry, addition.after);
    if (lines == nullptr)
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field, Json::LineAdditionMessage);
        return;
    }
    for (size_t index = 0; index < lines->size(); ++index)
    {
        const OrderedJson& lineJson = (*lines)[index];
        const std::string lineField = field + "." + Keys::Lines + "[" + std::to_string(index) + "]";
        if (!lineJson.is_object() || !lineJson.contains(Keys::Text))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, lineField, "must be an object with a text");
            continue;
        }
        if (lineJson.contains(Keys::After))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, lineField + "." + Keys::After,
                     "does not apply to an added line: it comes after the line its entry names");
        }
        ItemDescriptionLine line;
        if (ReadLine(lineJson, lineField, MaxDescriptionLineLength, line))
        {
            addition.lines.push_back(std::move(line));
        }
    }
    WarnAboutUnknownKeys(entry, {Keys::After, Keys::Lines}, field + ".");
    changes.add.push_back(std::move(addition));
}

// The fields of one line of a description or of the name lines; false when its text could not be read. A text longer
// than `maxLength` (MaxDescriptionLineLength, MaxNameLineLength) is a warning: the tooltip cuts it there.
bool ItemReader::ReadLine(const OrderedJson& entry, const std::string& field, size_t maxLength,
                          ItemDescriptionLine& line)
{
    bool read = true;
    if (const auto text = entry.find(Keys::Text); text != entry.end())
    {
        if (text->is_string() && text->get<std::string>().starts_with("@"))
        {
            // A shared text of the tooltip file; ValidateDescriptionTexts checks that it has it. "@" alone names none,
            // as in a layout.
            line.textName = text->get<std::string>().substr(1);
            if (line.textName.empty())
            {
                AddIssue(ItemDataIssueSeverity::Fatal, field + "." + Keys::Text,
                         "\"@\" names no text: a shared text is \"@\" and its name in the tooltip file's \"texts\"");
                read = false;
            }
        }
        else
        {
            read = Json::ReadLocalizedString(*text, field + "." + Keys::Text, line.text,
                                             [this](const std::string& errorField, const std::string& message)
                                             { AddIssue(ItemDataIssueSeverity::Fatal, errorField, message); });
            WarnAboutLongText(line.text, field + "." + Keys::Text, maxLength);
        }
    }
    if (const auto values = entry.find(Keys::Values); values != entry.end())
    {
        ReadDescriptionValues(*values, field + "." + Keys::Values, line);
    }
    if (const auto when = entry.find(Keys::When); when != entry.end())
    {
        ReadDescriptionConditions(*when, field + "." + Keys::When, line);
    }
    if (const auto color = entry.find(Keys::Color); color != entry.end())
    {
        ReadValue(*color, field + "." + Keys::Color, line.color);
    }
    if (const auto colors = entry.find(Keys::Colors); colors != entry.end())
    {
        ReadDescriptionColors(*colors, field + "." + Keys::Colors, line);
    }
    if (const auto after = entry.find(Keys::After); after != entry.end())
    {
        if (after->is_string() && after->get<std::string>() == Keys::AfterOptions)
        {
            line.afterOptions = true;
        }
        else
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field + "." + Keys::After,
                     "must be \"options\": the line is drawn after the item's options instead of with the "
                     "description");
        }
    }
    if (const auto bold = entry.find(Keys::Bold); bold != entry.end())
    {
        if (bold->is_boolean())
        {
            line.bold = bold->get<bool>();
        }
        else
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field + "." + Keys::Bold, "must be true or false");
        }
    }
    if (const auto levels = entry.find(Keys::Levels); levels != entry.end())
    {
        ReadDescriptionLevels(*levels, field + "." + Keys::Levels, line);
    }
    WarnAboutUnknownKeys(
        entry, {Keys::Text, Keys::Values, Keys::When, Keys::Color, Keys::Colors, Keys::Bold, Keys::Levels, Keys::After},
        field + ".");
    return read;
}

// A placeholder of a description line's text and the value it shows on the line: {"percent":
// "table:wingDamageIncrease1st"}. ValidateDescriptionPlaceholders checks the values' names.
void ItemReader::ReadDescriptionValues(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line)
{
    if (!json.is_object())
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field, "must be an object of placeholders and the values they show");
        return;
    }
    for (const auto& [placeholder, value] : json.items())
    {
        // The value is a built-in value's name, "table:" and a table's name, or a game data value
        // ("skillMana:<skill>").
        const std::string name = value.is_string() ? value.get<std::string>() : std::string();
        const std::string_view table = GetPlaceholderTable(name);
        if (!Json::IsName(placeholder) || !value.is_string() ||
            (!Json::IsName(table.empty() ? name : table) && !HasGameDataValuePrefix(name)))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field + "." + placeholder,
                     "must be a placeholder name and the name of the value it shows, both of letters and digits (a "
                     "table's as \"table:<name>\")");
            continue;
        }
        line.values.emplace_back(placeholder, value.get<std::string>());
    }
}

// {"stat": ["ENG", "STA", ...]}: the values of a description placeholder at the item levels from +0, each a text (as
// a name is written) or a shared text of the tooltip file ("@name").
void ItemReader::ReadLevelValues(const OrderedJson& json, std::vector<ItemLevelValues>& levelValues)
{
    const auto found = json.find(Keys::LevelValues);
    if (found == json.end())
    {
        return;
    }
    if (!found->is_object())
    {
        AddIssue(ItemDataIssueSeverity::Fatal, Keys::LevelValues, "must be an object of placeholders and their values");
        return;
    }
    for (const auto& [name, list] : found->items())
    {
        const std::string field = std::string(Keys::LevelValues) + "." + name;
        if (!Json::IsName(name))
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field, "a placeholder name has only letters and digits");
            continue;
        }
        if (!list.is_array() || list.empty() || list.size() > ItemLevelCount)
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field,
                     "must be a list of one value for each item level from +0, at most " +
                         std::to_string(ItemLevelCount));
            continue;
        }
        ItemLevelValues entry;
        entry.name = name;
        for (size_t level = 0; level < list.size(); ++level)
        {
            ItemLevelValue value;
            const OrderedJson& json = list[level];
            if (json.is_string() && json.get<std::string>().starts_with("@"))
            {
                // "@" alone names no shared text, as for a description line.
                value.textName = json.get<std::string>().substr(1);
                if (value.textName.empty())
                {
                    AddIssue(
                        ItemDataIssueSeverity::Fatal, field + "[" + std::to_string(level) + "]",
                        "\"@\" names no text: a shared text is \"@\" and its name in the tooltip file's \"texts\"");
                }
            }
            else
            {
                Json::ReadLocalizedString(json, field + "[" + std::to_string(level) + "]", value.text,
                                          [this](const std::string& errorField, const std::string& message)
                                          { AddIssue(ItemDataIssueSeverity::Fatal, errorField, message); });
            }
            entry.values.push_back(std::move(value));
        }
        levelValues.push_back(std::move(entry));
    }
}

// A list of conditions, such as ["durability", "!timed"]; false (with the problem added) when it is not a list.
bool ItemReader::ReadConditionList(const OrderedJson& json, const std::string& field,
                                   std::vector<TooltipCondition>& when)
{
    ReadTooltipConditions(json, field, R"(["durability", "!timed"])", when,
                          [this](const std::string& errorField, const std::string& message)
                          { AddIssue(ItemDataIssueSeverity::Fatal, errorField, message); });
    return json.is_array();
}

// The conditions a description line is shown on, all of them.
void ItemReader::ReadDescriptionConditions(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line)
{
    ReadConditionList(json, field, line.when);
}

// The line's color rules, [{"when": [...], "color": ...}, ...], as a layout line's.
void ItemReader::ReadDescriptionColors(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line)
{
    if (!json.is_array())
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field, "must be a list of {\"when\": [...], \"color\": ...}");
        return;
    }
    for (size_t index = 0; index < json.size(); ++index)
    {
        const OrderedJson& entry = json[index];
        const std::string entryField = field + "[" + std::to_string(index) + "]";
        const auto when = entry.is_object() ? entry.find(Keys::When) : entry.end();
        const auto color = entry.is_object() ? entry.find(Keys::Color) : entry.end();
        if (when == entry.end() || color == entry.end())
        {
            AddIssue(ItemDataIssueSeverity::Fatal, entryField, "must be {\"when\": [...], \"color\": ...}");
            continue;
        }
        TooltipColorRule rule;
        ReadConditionList(*when, entryField + "." + Keys::When, rule.when);
        if (rule.when.empty())
        {
            AddIssue(ItemDataIssueSeverity::Fatal, entryField + "." + Keys::When,
                     "a color rule needs a condition; the line's \"color\" is the color when no rule holds");
        }
        ReadValue(*color, entryField + "." + Keys::Color, rule.color);
        WarnAboutUnknownKeys(entry, {Keys::When, Keys::Color}, entryField + ".");
        line.colors.push_back(std::move(rule));
    }
}

// [first, last]: item levels, both included; false (with the problem added) for anything else.
bool ItemReader::ReadLevelRange(const OrderedJson& json, const std::string& field, int& first, int& last)
{
    // Read as a whole number first: a number outside int's range would wrap into a level.
    const auto readLevel = [](const OrderedJson& value, long long& level)
    { return ReadWholeNumber(value, level) && level >= 0 && level < ItemLevelCount; };
    long long firstLevel = 0;
    long long lastLevel = 0;
    if (!json.is_array() || json.size() != 2 || !readLevel(json[0], firstLevel) || !readLevel(json[1], lastLevel) ||
        firstLevel > lastLevel)
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field,
                 "must be [first, last], two item levels from 0 to " + std::to_string(ItemLevelCount - 1) +
                     " with the first not above the last");
        return false;
    }
    first = static_cast<int>(firstLevel);
    last = static_cast<int>(lastLevel);
    return true;
}

// [first, last]: the item levels a description line is shown at, both included.
void ItemReader::ReadDescriptionLevels(const OrderedJson& json, const std::string& field, ItemDescriptionLine& line)
{
    ReadLevelRange(json, field, line.minLevel, line.maxLevel);
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
        AddIssue(ItemDataIssueSeverity::Fatal, Keys::Requirements, "must be an object");
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
        AddIssue(ItemDataIssueSeverity::Fatal, key, "must be an object");
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

// UTF-16 characters, as the client's wide strings hold the text: a character past U+FFFF counts as two.
static size_t Utf16Length(std::string_view text)
{
    size_t length = 0;
    for (const char c : text)
    {
        const auto byte = static_cast<unsigned char>(c);
        if ((byte & 0xC0) != 0x80)
        {
            length += byte >= 0xF0 ? 2 : 1;
        }
    }
    return length;
}

void ItemReader::WarnAboutLongText(const LocalizedString& text, const std::string& field, size_t maxLength)
{
    const auto check = [&](std::string_view locale, const std::string& written)
    {
        if (Utf16Length(written) > maxLength)
        {
            AddIssue(ItemDataIssueSeverity::Warning, field,
                     "is longer than " + std::to_string(maxLength) + " characters in \"" + std::string(locale) +
                         "\": the tooltip shows only the first " + std::to_string(maxLength));
        }
    };
    check(LocalizedString::NeutralLocale, text.GetNeutral());
    for (const auto& [locale, translation] : text.GetTranslations())
    {
        check(locale, translation);
    }
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
        AddIssue(ItemDataIssueSeverity::Fatal, Keys::RequirementsByLevel, "must be an object");
        return;
    }
    for (const auto& [key, values] : byLevel->items())
    {
        const std::string field = std::string(Keys::RequirementsByLevel) + "." + key;
        const auto requirement = std::find(RequirementKeys.begin(), RequirementKeys.end(), key);
        if (requirement == RequirementKeys.end())
        {
            AddIssue(ItemDataIssueSeverity::Fatal, field, "is not a requirement");
            continue;
        }
        RequirementByLevel levels{};
        const bool read = Json::ReadPerLevelValues(
            values, field, levels, [this](const OrderedJson& entry, const std::string& entryField, WORD& value)
            { ReadValue(entry, entryField, value); }, [this](const std::string& errorField, const std::string& message)
            { AddIssue(ItemDataIssueSeverity::Fatal, errorField, message); });
        if (read)
        {
            definition.requirementsByLevel.Set(static_cast<ItemRequirement>(requirement - RequirementKeys.begin()),
                                               levels);
        }
    }
}

std::set<std::string, std::less<>> GetKnownItemKeys()
{
    std::set<std::string, std::less<>> keys{
        Keys::Number,       Keys::Name,          Keys::NameForms,           Keys::Tags,
        Keys::Options,      Keys::Description,   Keys::LevelValues,         Keys::NameLines,
        Keys::GroundLabels, Keys::Requirements,  Keys::RequirementsByLevel, Keys::ClassRequirements,
        Keys::Resistances,  Keys::TooltipLayout, Keys::TooltipLayoutChanges};
    ItemDefinition unused;
    VisitStatFields(unused, [&](const char* key, auto&, const auto&) { keys.insert(key); });
    return keys;
}

bool ItemReader::Read(const OrderedJson& json, ItemDefinition& definition)
{
    if (!json.is_object())
    {
        AddIssue(ItemDataIssueSeverity::Fatal, "", "an item must be an object");
        return false;
    }
    if (!ReadIdentity(json, definition))
    {
        return false;
    }

    if (const auto forms = json.find(Keys::NameForms); forms != json.end())
    {
        ReadNameForms(*forms, Keys::NameForms, definition.nameForms);
    }
    ReadTags(json, definition.tags);
    ReadOptionLinks(json, definition.options);
    ReadDescription(json, definition.description);
    ReadLevelValues(json, definition.levelValues);
    ReadNameLines(json, definition.nameLines);
    ReadGroundLabels(json, definition.groundLabels);
    if (const auto layout = json.find(Keys::TooltipLayout); layout != json.end())
    {
        ReadValue(*layout, Keys::TooltipLayout, definition.tooltipLayout);
        // The layout of an item that names none, so it is read as none: a removed tag's preset stands in for it, and a
        // save leaves it out.
        if (definition.tooltipLayout == DefaultTooltipLayout)
        {
            definition.tooltipLayout.clear();
        }
    }
    ReadTooltipLayoutChanges(json, definition.tooltipLayoutChanges);
    ApplyRemovedTooltipTags(definition);
    ReadStats(json, definition);
    ReadRequirements(json, definition);
    ReadRequirementsByLevel(json, definition);
    ReadByteTable(json, Keys::ClassRequirements, definition.classRequirements, ClassKeys);
    ReadByteTable(json, Keys::Resistances, definition.resistances, ResistanceKeys);

    static const std::set<std::string, std::less<>> KnownItemKeys = GetKnownItemKeys();
    WarnAboutUnknownKeys(json, KnownItemKeys, "");
    return !m_hasFatal;
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
        // An entry without a name is a number the client uses without an item there.
        ItemReader reader(source, group, issues, false);
        if (reader.Read(json, definition))
        {
            // Only without the name key: an empty name is still an item without its English name.
            definition.entryWithoutName = !json.contains(Keys::Name);
            items.push_back(std::move(definition));
        }
    }
}

std::span<const char* const> GetClassKeys()
{
    return ClassKeys;
}

std::span<const RemovedTooltipTag> GetRemovedTooltipTags()
{
    return RemovedTooltipTags;
}

const RemovedTooltipTag* FindRemovedTooltipTag(std::string_view tag)
{
    const auto removed = std::find_if(std::begin(RemovedTooltipTags), std::end(RemovedTooltipTags),
                                      [&](const RemovedTooltipTag& entry) { return tag == entry.tag; });
    return removed != std::end(RemovedTooltipTags) ? &*removed : nullptr;
}

// Also what the item reader takes the tag as.
std::string DescribeRemovedTooltipTag(const RemovedTooltipTag& removed)
{
    if (!removed.preset.empty())
    {
        std::string text = R"("tooltipLayout": ")" + std::string(removed.preset) + "\"";
        if (!removed.bothPreset.empty())
        {
            text += R"( (with both noClassLines and noDurabilityLine: ")" + std::string(removed.bothPreset) + "\")";
        }
        return text;
    }
    std::string ids;
    for (const std::string_view id : removed.hide)
    {
        ids += (ids.empty() ? "\"" : ", \"") + std::string(id) + "\"";
    }
    return R"("tooltipLayoutChanges": {"hide": [)" + ids + "]}";
}

std::string WriteItemGroupJson(int group, std::span<const ItemDefinition> items)
{
    std::vector<const ItemDefinition*> groupItems;
    for (const ItemDefinition& definition : items)
    {
        if (definition.group == group && definition.HasEntry())
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
                  std::vector<ItemDataIssue>& issues, bool nameRequired)
{
    ItemReader reader(source, group, issues, nameRequired);
    return reader.Read(json, definition);
}

std::string LayOutItemJsonText(const std::string& text)
{
    std::string laidOut = Json::PutListsOnOneLine(text, Keys::Tags);
    laidOut = Json::PutListsOnOneLine(laidOut, Keys::Options);
    laidOut = Json::PutListsOnOneLine(laidOut, Keys::GroundLabels);
    laidOut = Json::PutListsOnOneLine(laidOut, Keys::Hide);
    laidOut = Json::PutListsOnOneLine(laidOut, Keys::Levels);
    laidOut = Json::PutListsOnOneLine(laidOut, Keys::When);
    laidOut = Json::PutObjectsOnOneLine(laidOut, Keys::Values);
    return Json::PutObjectsOnOneLine(laidOut, Keys::RequirementsByLevel);
}
} // namespace Data::Items
