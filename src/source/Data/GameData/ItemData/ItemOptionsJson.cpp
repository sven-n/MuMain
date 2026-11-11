#include "stdafx.h"

#include "ItemOptionsJson.h"
#include "ItemEnumNames.h"
#include "ItemJsonCommon.h"

#include <array>
#include <cctype>
#include <set>

namespace Data::Items
{
namespace
{
using Json::KeySet;
using Json::OrderedJson;

namespace Keys
{
constexpr const char* Definitions = "definitions";
constexpr const char* Name = "name";
constexpr const char* Type = "type";
constexpr const char* DisplayName = "displayName";
constexpr const char* OpenMu = "openMu";
constexpr const char* Id = "id";
constexpr const char* Server = "server";
constexpr const char* AddsRandomly = "addsRandomly";
constexpr const char* AddChance = "addChance";
constexpr const char* MaximumOptionsPerItem = "maximumOptionsPerItem";
constexpr const char* Options = "options";
constexpr const char* Number = "number";
constexpr const char* Attribute = "attribute";
constexpr const char* Text = "text";
constexpr const char* Values = "values";
constexpr const char* PerLevel = "perLevel";
} // namespace Keys

// The keys of the ways a value adds, in the order of ItemOptionAdd.
constexpr std::array<const char*, 3> AddKeys = {"add", "addFinal", "percent"};

constexpr int MaxValue = 65535;
constexpr int MaxNumber = 255;
// The additional option's number is the option type the server sends, the option byte's high four bits.
constexpr int MaxOptionTypeNumber = 15;
constexpr int MaxOptionsPerItem = 255;
// OpenMU's GUIDs: 8, 4, 4, 4 and 12 hexadecimal digits.
constexpr std::array<size_t, 5> GuidGroupLengths = {8, 4, 4, 4, 12};

const KeySet RootKeys = {Json::Keys::FormatVersion, Keys::Definitions};
const KeySet DefinitionKeys = {Keys::Name, Keys::Type, Keys::DisplayName, Keys::OpenMu, Keys::Server, Keys::Options};
const KeySet OpenMuKeys = {Keys::Id, Keys::Name};
const KeySet ServerKeys = {Keys::AddsRandomly, Keys::AddChance, Keys::MaximumOptionsPerItem};
const KeySet OptionKeys = {Keys::Number, Keys::Attribute, AddKeys[0], AddKeys[1], AddKeys[2], Keys::Text};
// perLevel has an error of its own, so it is no unknown field.
const KeySet ValueKeys = {Keys::Values, Keys::PerLevel};

// Whether the options of a type have a number (OpenMU's ItemOption.Number), and whether a definition of it has one
// option only.
bool HasNumbers(ItemOptionType type)
{
    return type != ItemOptionType::Luck;
}

bool HasOneOption(ItemOptionType type)
{
    return type == ItemOptionType::Luck;
}

bool IsGuid(std::string_view text)
{
    size_t position = 0;
    for (size_t group = 0; group < GuidGroupLengths.size(); ++group)
    {
        if (group > 0)
        {
            if (position >= text.size() || text[position] != '-')
            {
                return false;
            }
            ++position;
        }
        for (size_t digit = 0; digit < GuidGroupLengths[group]; ++digit, ++position)
        {
            if (position >= text.size() || !std::isxdigit(static_cast<unsigned char>(text[position])))
            {
                return false;
            }
        }
    }
    return position == text.size();
}

class ItemOptionsReader
{
public:
    ItemOptionsReader(const std::string& source, std::vector<ItemDataIssue>& issues)
        : m_source(source), m_issues(issues)
    {
    }

    bool ReadDefinition(const OrderedJson& json, const std::string& field, ItemOptionDefinition& definition);

private:
    void AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    {
        m_issues.push_back({severity, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
        m_hasFatal = m_hasFatal || severity == ItemDataIssueSeverity::Fatal;
    }

    void AddFatal(const std::string& field, const std::string& message)
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field, message);
    }

    void WarnAboutUnknownKeys(const OrderedJson& json, const KeySet& knownKeys, const std::string& field);
    bool ReadName(const OrderedJson& json, const char* key, const std::string& field, std::string& name);
    void ReadType(const OrderedJson& json, const std::string& field, ItemOptionType& type);
    void ReadDisplayName(const OrderedJson& json, const std::string& field, LocalizedString& displayName);
    void ReadOpenMu(const OrderedJson& json, const std::string& field, ItemOptionOpenMuLabels& labels);
    void ReadServer(const OrderedJson& json, const std::string& field, std::optional<ItemOptionServerValues>& server);
    void ReadOptions(const OrderedJson& json, const std::string& field, ItemOptionDefinition& definition);
    void ReadOption(const OrderedJson& json, const std::string& field, ItemOptionType type, ItemOption& option);
    void ReadNumber(const OrderedJson& json, const std::string& field, ItemOptionType type, ItemOption& option);
    void ReadAdd(const OrderedJson& json, const std::string& field, ItemOption& option);
    void ReadValue(const OrderedJson& json, const std::string& field, ItemOptionValue& value);
    bool ReadWholeNumber(const OrderedJson& json, const std::string& field, int minimum, int maximum, int& value);
    void ReadTexts(const OrderedJson& json, const std::string& field, std::vector<std::string>& texts);

    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
    bool m_hasFatal = false;
};

void ItemOptionsReader::WarnAboutUnknownKeys(const OrderedJson& json, const KeySet& knownKeys, const std::string& field)
{
    Json::WarnAboutUnknownKeys(json, knownKeys, field + ".",
                               [this](const std::string& unknown, const std::string& message)
                               { AddIssue(ItemDataIssueSeverity::Warning, unknown, message); });
}

bool ItemOptionsReader::ReadName(const OrderedJson& json, const char* key, const std::string& field, std::string& name)
{
    return Json::ReadNameField(json, key, field, name, [this](const std::string& errorField, const std::string& message)
                               { AddFatal(errorField, message); });
}

void ItemOptionsReader::ReadType(const OrderedJson& json, const std::string& field, ItemOptionType& type)
{
    const auto value = json.find(Keys::Type);
    if (value == json.end() || !value->is_string() || !FindEnumValue(value->get_ref<const std::string&>(), type))
    {
        AddFatal(field + "." + Keys::Type, "missing or not a type; the types are " + JoinEnumNames<ItemOptionType>());
    }
}

void ItemOptionsReader::ReadDisplayName(const OrderedJson& json, const std::string& field, LocalizedString& displayName)
{
    const std::string nameField = field + "." + Keys::DisplayName;
    const auto value = json.find(Keys::DisplayName);
    if (value == json.end())
    {
        AddFatal(nameField, "missing: a definition has a name by language, the English one first");
        return;
    }
    if (!Json::ReadLocalizedString(*value, nameField, displayName,
                                   [this](const std::string& errorField, const std::string& message)
                                   { AddFatal(errorField, message); }))
    {
        return;
    }
    if (displayName.GetNeutral().empty())
    {
        AddFatal(nameField, "the English name is missing");
    }
    for (const auto& [locale, translation] : displayName.GetTranslations())
    {
        if (!LocalizedString::IsValidLocaleCode(locale))
        {
            AddFatal(nameField, "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
        }
    }
}

void ItemOptionsReader::ReadOpenMu(const OrderedJson& json, const std::string& field, ItemOptionOpenMuLabels& labels)
{
    const auto value = json.find(Keys::OpenMu);
    if (value == json.end())
    {
        return;
    }
    const std::string labelsField = field + "." + Keys::OpenMu;
    if (!value->is_object())
    {
        AddFatal(labelsField, R"(must be {"id": "<OpenMU's GUID>", "name": "<OpenMU's English name>"})");
        return;
    }
    if (const auto id = value->find(Keys::Id); id != value->end())
    {
        if (!id->is_string() || !IsGuid(id->get_ref<const std::string&>()))
        {
            AddFatal(labelsField + "." + Keys::Id, "must be a GUID: \"00000083-0001-0000-0000-000000000000\"");
        }
        else
        {
            labels.id = id->get<std::string>();
        }
    }
    if (const auto name = value->find(Keys::Name); name != value->end())
    {
        if (!name->is_string() || name->get_ref<const std::string&>().empty())
        {
            AddFatal(labelsField + "." + Keys::Name, "must be OpenMU's English name of the definition");
        }
        else
        {
            labels.name = name->get<std::string>();
        }
    }
    WarnAboutUnknownKeys(*value, OpenMuKeys, labelsField);
}

void ItemOptionsReader::ReadServer(const OrderedJson& json, const std::string& field,
                                   std::optional<ItemOptionServerValues>& server)
{
    const auto value = json.find(Keys::Server);
    if (value == json.end())
    {
        return;
    }
    const std::string serverField = field + "." + Keys::Server;
    if (!value->is_object())
    {
        AddFatal(serverField, R"(must be {"addsRandomly": true, "addChance": 0.25, "maximumOptionsPerItem": 1})");
        return;
    }
    ItemOptionServerValues values;
    if (const auto addsRandomly = value->find(Keys::AddsRandomly); addsRandomly != value->end())
    {
        if (addsRandomly->is_boolean())
        {
            values.addsRandomly = addsRandomly->get<bool>();
        }
        else
        {
            AddFatal(serverField + "." + Keys::AddsRandomly, "must be true or false");
        }
    }
    if (const auto addChance = value->find(Keys::AddChance); addChance != value->end())
    {
        if (addChance->is_number() && addChance->get<double>() >= 0.0 && addChance->get<double>() <= 1.0)
        {
            values.addChance = addChance->get<double>();
        }
        else
        {
            AddFatal(serverField + "." + Keys::AddChance, "must be a number from 0 to 1");
        }
    }
    if (const auto maximum = value->find(Keys::MaximumOptionsPerItem); maximum != value->end())
    {
        ReadWholeNumber(*maximum, serverField + "." + Keys::MaximumOptionsPerItem, 0, MaxOptionsPerItem,
                        values.maximumOptionsPerItem);
    }
    WarnAboutUnknownKeys(*value, ServerKeys, serverField);
    server = values;
}

bool ItemOptionsReader::ReadWholeNumber(const OrderedJson& json, const std::string& field, int minimum, int maximum,
                                        int& value)
{
    return Json::ReadWholeNumberInRange(json, field, minimum, maximum, value,
                                        [this](const std::string& errorField, const std::string& message)
                                        { AddFatal(errorField, message); });
}

void ItemOptionsReader::ReadValue(const OrderedJson& json, const std::string& field, ItemOptionValue& value)
{
    if (!json.is_object())
    {
        AddFatal(field, R"(must be an object: {"values": 5}, or {"values": [4, 8, 12, 16]} for one value per level)");
        return;
    }
    if (json.contains(Keys::PerLevel))
    {
        AddFatal(field + "." + Keys::PerLevel,
                 R"(is not a value; write one value per level as a list: {"values": [4, 8, 12, 16]})");
    }
    const auto values = json.find(Keys::Values);
    if (values == json.end() || (!values->is_number() && (!values->is_array() || values->empty())))
    {
        AddFatal(field + "." + Keys::Values, "missing, or neither a number nor a list of one value per level");
        return;
    }
    value.isList = values->is_array();
    if (!value.isList)
    {
        int constant = 0;
        ReadWholeNumber(*values, field + "." + Keys::Values, 0, MaxValue, constant);
        value.values = {constant};
    }
    else
    {
        for (size_t index = 0; index < values->size(); ++index)
        {
            int entry = 0;
            ReadWholeNumber((*values)[index], field + "." + Keys::Values + "[" + std::to_string(index) + "]", 0,
                            MaxValue, entry);
            value.values.push_back(entry);
        }
    }
    WarnAboutUnknownKeys(json, ValueKeys, field);
}

void ItemOptionsReader::ReadTexts(const OrderedJson& json, const std::string& field, std::vector<std::string>& texts)
{
    constexpr const char* Message = "must be the name of a text of OptionTexts.json, or a list of them";
    if (json.is_string() && Json::IsName(json.get_ref<const std::string&>()))
    {
        texts.push_back(json.get<std::string>());
        return;
    }
    if (!json.is_array() || json.empty())
    {
        AddFatal(field, Message);
        return;
    }
    for (const OrderedJson& entry : json)
    {
        if (!entry.is_string() || !Json::IsName(entry.get_ref<const std::string&>()))
        {
            AddFatal(field, Message);
            return;
        }
        texts.push_back(entry.get<std::string>());
    }
}

void ItemOptionsReader::ReadNumber(const OrderedJson& json, const std::string& field, ItemOptionType type,
                                   ItemOption& option)
{
    const auto number = json.find(Keys::Number);
    if (!HasNumbers(type))
    {
        if (number != json.end())
        {
            AddFatal(field + "." + Keys::Number,
                     std::string("a ") + FindEnumName(type) + " option has no number; leave it out");
        }
        return;
    }
    if (number == json.end())
    {
        AddFatal(field + "." + Keys::Number, "missing");
        return;
    }
    ReadWholeNumber(*number, field + "." + Keys::Number, 0,
                    type == ItemOptionType::Option ? MaxOptionTypeNumber : MaxNumber, option.number);
}

void ItemOptionsReader::ReadAdd(const OrderedJson& json, const std::string& field, ItemOption& option)
{
    const OrderedJson* value = nullptr;
    for (size_t add = 0; add < AddKeys.size(); ++add)
    {
        const auto found = json.find(AddKeys[add]);
        if (found == json.end())
        {
            continue;
        }
        if (value != nullptr)
        {
            AddFatal(field, R"(has more than one of "add", "addFinal" and "percent")");
            return;
        }
        value = &*found;
        option.add = static_cast<ItemOptionAdd>(add);
    }
    if (value == nullptr)
    {
        AddFatal(field, R"(needs its value as "add", "addFinal" or "percent")");
        return;
    }
    ReadValue(*value, field + "." + AddKeys[static_cast<size_t>(option.add)], option.value);
}

void ItemOptionsReader::ReadOption(const OrderedJson& json, const std::string& field, ItemOptionType type,
                                   ItemOption& option)
{
    if (!json.is_object())
    {
        AddFatal(field, "an option must be an object");
        return;
    }
    ReadNumber(json, field, type, option);
    ReadName(json, Keys::Attribute, field, option.attribute);
    ReadAdd(json, field, option);
    const auto text = json.find(Keys::Text);
    if (text == json.end())
    {
        AddFatal(field + "." + Keys::Text, "missing: an option names the texts of its tooltip lines");
    }
    else
    {
        ReadTexts(*text, field + "." + Keys::Text, option.texts);
    }
    WarnAboutUnknownKeys(json, OptionKeys, field);
}

void ItemOptionsReader::ReadOptions(const OrderedJson& json, const std::string& field, ItemOptionDefinition& definition)
{
    const std::string optionsField = field + "." + Keys::Options;
    const auto options = json.find(Keys::Options);
    if (options == json.end() || !options->is_array() || options->empty())
    {
        AddFatal(optionsField, "missing, or not a list of one or more options");
        return;
    }
    if (HasOneOption(definition.type) && options->size() > 1)
    {
        AddFatal(optionsField, std::string("a ") + FindEnumName(definition.type) + " definition has one option");
    }
    std::set<int> numbers;
    for (size_t index = 0; index < options->size(); ++index)
    {
        const std::string optionField = optionsField + "[" + std::to_string(index) + "]";
        ItemOption option;
        ReadOption((*options)[index], optionField, definition.type, option);
        if (HasNumbers(definition.type) && !numbers.insert(option.number).second)
        {
            AddFatal(optionField + "." + Keys::Number,
                     std::to_string(option.number) + " is the number of an option before it");
        }
        definition.options.push_back(std::move(option));
    }
}

bool ItemOptionsReader::ReadDefinition(const OrderedJson& json, const std::string& field,
                                       ItemOptionDefinition& definition)
{
    if (!json.is_object())
    {
        AddFatal(field, "a definition must be an object");
        return false;
    }
    m_hasFatal = false;
    ReadName(json, Keys::Name, field, definition.name);
    ReadType(json, field, definition.type);
    ReadDisplayName(json, field, definition.displayName);
    ReadOpenMu(json, field, definition.openMu);
    ReadServer(json, field, definition.server);
    ReadOptions(json, field, definition);
    WarnAboutUnknownKeys(json, DefinitionKeys, field);
    return !m_hasFatal;
}

// ---------------------------------------------------------------- writing

OrderedJson WriteValue(const ItemOptionValue& value)
{
    OrderedJson json;
    if (value.isList)
    {
        json[Keys::Values] = value.values;
    }
    else
    {
        json[Keys::Values] = value.values.empty() ? 0 : value.values.front();
    }
    return json;
}

OrderedJson WriteOption(const ItemOption& option, ItemOptionType type)
{
    OrderedJson json;
    if (HasNumbers(type))
    {
        json[Keys::Number] = option.number;
    }
    json[Keys::Attribute] = option.attribute;
    json[AddKeys[static_cast<size_t>(option.add)]] = WriteValue(option.value);
    json[Keys::Text] = option.texts.size() == 1 ? OrderedJson(option.texts.front()) : OrderedJson(option.texts);
    return json;
}

OrderedJson WriteDefinition(const ItemOptionDefinition& definition)
{
    OrderedJson json;
    json[Keys::Name] = definition.name;
    const char* type = FindEnumName(definition.type);
    json[Keys::Type] = type != nullptr ? OrderedJson(type) : OrderedJson(static_cast<int>(definition.type));
    json[Keys::DisplayName] = Json::WriteLocalizedString(definition.displayName);
    if (!definition.openMu.IsEmpty())
    {
        OrderedJson labels;
        if (!definition.openMu.id.empty())
        {
            labels[Keys::Id] = definition.openMu.id;
        }
        if (!definition.openMu.name.empty())
        {
            labels[Keys::Name] = definition.openMu.name;
        }
        json[Keys::OpenMu] = std::move(labels);
    }
    if (definition.server)
    {
        OrderedJson server;
        server[Keys::AddsRandomly] = definition.server->addsRandomly;
        server[Keys::AddChance] = definition.server->addChance;
        server[Keys::MaximumOptionsPerItem] = definition.server->maximumOptionsPerItem;
        json[Keys::Server] = std::move(server);
    }
    json[Keys::Options] = OrderedJson::array();
    for (const ItemOption& option : definition.options)
    {
        json[Keys::Options].push_back(WriteOption(option, definition.type));
    }
    return json;
}
} // namespace

void ReadItemOptionsJson(std::string_view text, const std::string& source, const std::string& fileName,
                         std::vector<ItemOptionDefinition>& definitions, std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, ItemOptionsFormatVersion, root, issues))
    {
        return;
    }
    const auto list = root.find(Keys::Definitions);
    if (list == root.end() || !list->is_array())
    {
        Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::Definitions,
                           "missing or not a list of option definitions");
        return;
    }
    ItemOptionsReader reader(source, issues);
    for (size_t index = 0; index < list->size(); ++index)
    {
        ItemOptionDefinition definition;
        if (reader.ReadDefinition((*list)[index], std::string(Keys::Definitions) + "[" + std::to_string(index) + "]",
                                  definition))
        {
            definition.file = fileName;
            definitions.push_back(std::move(definition));
        }
    }
    Json::WarnAboutUnknownKeys(root, RootKeys, "",
                               [&](const std::string& unknown, const std::string& message)
                               {
                                   issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem,
                                                     ItemDataIssue::NoItem, unknown, message});
                               });
}

std::string WriteItemOptionsJson(std::span<const ItemOptionDefinition> definitions)
{
    OrderedJson root;
    root[Json::Keys::FormatVersion] = ItemOptionsFormatVersion;
    root[Keys::Definitions] = OrderedJson::array();
    for (const ItemOptionDefinition& definition : definitions)
    {
        root[Keys::Definitions].push_back(WriteDefinition(definition));
    }
    std::string text = root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    text = Json::PutObjectsOnOneLine(text, Keys::OpenMu);
    text = Json::PutObjectsOnOneLine(text, Keys::Server);
    return Json::PutListEntriesOnOneLine(text, Keys::Options) + "\n";
}
} // namespace Data::Items
