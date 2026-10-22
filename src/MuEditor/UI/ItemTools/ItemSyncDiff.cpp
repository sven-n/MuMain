#include "stdafx.h"

#ifdef _EDITOR

#include "ItemSyncDiff.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemStatsJson.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <map>
#include <utility>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Items::ItemDefinition;
using Data::Items::ItemExchangeFile;
using Data::Items::Json::OrderedJson;

// A side's fields by their path, as JSON.
using Fields = std::map<std::string, OrderedJson, std::less<>>;
using ItemKey = std::pair<int, int>;

constexpr int KnownDifferencesFormatVersion = 1;
constexpr const char* NameField = "name";
constexpr const char* TagsField = "tags";
constexpr const char* LevelField = "level";
// A drop level OpenMU can keep: its field is a byte.
constexpr int OpenMUMaxDropLevel = 255;
// What the game holds keeps (IsHeldByCharacter).
constexpr const char* HeldFields[] = {"width", "height", "slot"};

std::string ToText(const OrderedJson& value)
{
    return value.dump(-1, ' ', false, OrderedJson::error_handler_t::replace);
}

std::optional<std::string> TextOf(const Fields& fields, const std::string& path)
{
    const auto found = fields.find(path);
    return found != fields.end() ? std::optional<std::string>(ToText(found->second)) : std::nullopt;
}

// ---------------------------------------------------------------- items

Fields FlattenItem(const ItemDefinition& item)
{
    Fields fields;
    const OrderedJson json = Data::Items::WriteItemJson(item);
    for (const auto& [key, value] : json.items())
    {
        if (key == Data::Items::Json::Keys::Number)
        {
            continue;
        }
        if (key == TagsField)
        {
            for (const OrderedJson& tag : value)
            {
                fields[std::string(TagsField) + "." + tag.get<std::string>()] = true;
            }
            continue;
        }
        if (value.is_object())
        {
            for (const auto& [part, partValue] : value.items())
            {
                fields[key + "." + part] = partValue;
            }
            continue;
        }
        fields[key] = value;
    }
    return fields;
}

ItemDefinition UnflattenItem(int group, int number, const Fields& fields, std::vector<ItemDataIssue>& issues)
{
    OrderedJson json;
    json[Data::Items::Json::Keys::Number] = number;
    for (const auto& [path, value] : fields)
    {
        const size_t dot = path.find('.');
        if (dot == std::string::npos)
        {
            json[path] = value;
            continue;
        }
        const std::string top = path.substr(0, dot);
        const std::string part = path.substr(dot + 1);
        if (top == TagsField)
        {
            json[top].push_back(part);
        }
        else
        {
            json[top][part] = value;
        }
    }
    ItemDefinition item;
    Data::Items::ReadItemJson(json, "", group, item, issues);
    return Data::Items::ToExchangeItem(item);
}

bool IsNamePath(const std::string& path, std::string& locale)
{
    const std::string prefix = std::string(NameField) + ".";
    if (path.rfind(prefix, 0) != 0)
    {
        return false;
    }
    locale = path.substr(prefix.size());
    return true;
}

// Whether `path` of the incoming side takes part: a language the incoming side lacks stays,
// and an import takes names only in the client's UI languages.
bool TakesPart(const std::string& path, bool incomingHasIt, const SyncOptions& options)
{
    std::string locale;
    if (!IsNamePath(path, locale))
    {
        return true;
    }
    if (!incomingHasIt)
    {
        return false;
    }
    return options.direction != SyncDirection::Import ||
           std::ranges::find(options.uiLocales, locale) != options.uiLocales.end();
}

std::map<ItemKey, const ItemDefinition*> IndexItems(const ItemExchangeFile& file)
{
    std::map<ItemKey, const ItemDefinition*> items;
    for (const ItemDefinition& item : file.items)
    {
        items[{item.group, item.number}] = &item;
    }
    return items;
}

bool IsHeld(const SyncOptions& options, const ItemKey& key)
{
    return options.direction == SyncDirection::Import && options.isHeld &&
           options.isHeld(Data::Items::MakeItemType(key.first, key.second));
}

void MarkField(SyncField& field, const ItemKey& key, const ItemExchangeFile& incoming, const Fields& current,
               const Fields& incomingFields, const SyncOptions& options)
{
    field.known = options.known != nullptr && options.known->Contains(key.first, key.second, field.field);
    if (IsHeld(options, key) && std::ranges::find(HeldFields, field.field) != std::end(HeldFields))
    {
        field.locked = "the game holds the item: its size and slot stay";
    }
    if (field.field == LevelField && options.direction == SyncDirection::Import &&
        incoming.source == Data::Items::ItemExchangeSourceOpenMU)
    {
        const auto client = current.find(LevelField);
        const auto server = incomingFields.find(LevelField);
        if (client != current.end() && client->second.get<int>() > OpenMUMaxDropLevel &&
            server != incomingFields.end() && server->second.get<int>() == OpenMUMaxDropLevel)
        {
            field.inexact = "OpenMU keeps the drop level in a byte (at most 255)";
        }
    }
    field.checked = !field.known && field.locked.empty() && field.inexact.empty();
}

SyncItem CompareItem(const ItemKey& key, const ItemDefinition* current, const ItemDefinition* incoming,
                     const ItemExchangeFile& incomingFile, const SyncOptions& options)
{
    SyncItem row;
    row.group = key.first;
    row.number = key.second;
    row.name = (incoming != nullptr ? incoming : current)->names.GetNeutral();
    row.known = options.known != nullptr && options.known->Contains(key.first, key.second, "");
    const Fields currentFields = current != nullptr ? FlattenItem(*current) : Fields{};
    const Fields incomingFields = incoming != nullptr ? FlattenItem(*incoming) : Fields{};

    if (current == nullptr)
    {
        row.change = SyncChange::New;
        for (const auto& [path, value] : incomingFields)
        {
            if (TakesPart(path, true, options))
            {
                row.fields.push_back({path, std::nullopt, ToText(value)});
            }
        }
        row.checked = !row.known;
        return row;
    }
    if (incoming == nullptr)
    {
        row.change = SyncChange::Removed;
        if (IsHeld(options, key))
        {
            row.locked = "the game holds the item: it is not removed";
        }
        return row;
    }

    row.change = SyncChange::Changed;
    std::set<std::string, std::less<>> paths;
    for (const auto& [path, value] : currentFields)
    {
        paths.insert(path);
    }
    for (const auto& [path, value] : incomingFields)
    {
        paths.insert(path);
    }
    for (const std::string& path : paths)
    {
        const std::optional<std::string> was = TextOf(currentFields, path);
        const std::optional<std::string> becomes = TextOf(incomingFields, path);
        if (was == becomes || !TakesPart(path, becomes.has_value(), options))
        {
            continue;
        }
        SyncField field{path, was, becomes};
        MarkField(field, key, incomingFile, currentFields, incomingFields, options);
        row.fields.push_back(std::move(field));
    }
    return row;
}

// ---------------------------------------------------------------- formulas

constexpr const char* DropLevelBonusEntry = "dropLevelBonus";
constexpr const char* TablesEntry = "tables";
constexpr const char* DefaultEntry = "default";
constexpr const char* ProfilesEntry = "profiles";
constexpr const char* RequirementsKey = "requirements";

Fields FlattenFormulas(const Data::Items::ItemStatFile& file)
{
    Fields fields;
    const OrderedJson json = Data::Items::WriteItemStatFormulasJson(file);
    for (const auto& [key, value] : json.at(DropLevelBonusEntry).items())
    {
        fields[std::string(DropLevelBonusEntry) + "." + key] = value;
    }
    for (const OrderedJson& table : json.at(TablesEntry))
    {
        OrderedJson values = table;
        values.erase(NameField);
        fields[std::string(TablesEntry) + "." + table.at(NameField).get<std::string>()] = std::move(values);
    }
    for (const auto& [stat, section] : json.at(DefaultEntry).items())
    {
        for (const auto& [term, value] : section.items())
        {
            fields[std::string(DefaultEntry) + "." + stat + "." + term] = value;
        }
    }
    for (const OrderedJson& profile : json.at(ProfilesEntry))
    {
        const std::string prefix = std::string(ProfilesEntry) + "." + profile.at(NameField).get<std::string>();
        fields[prefix] = true;
        for (const auto& [stat, section] : profile.items())
        {
            if (stat == NameField)
            {
                continue;
            }
            for (const auto& [term, value] : section.items())
            {
                fields[prefix + "." + stat + "." + term] = value;
            }
        }
    }
    return fields;
}

std::vector<std::string> SplitPath(const std::string& path)
{
    std::vector<std::string> parts;
    size_t start = 0;
    while (true)
    {
        const size_t dot = path.find('.', start);
        parts.push_back(path.substr(start, dot - start));
        if (dot == std::string::npos)
        {
            return parts;
        }
        start = dot + 1;
    }
}

std::optional<Data::Items::ItemStatFile> UnflattenFormulas(const Fields& fields, std::vector<ItemDataIssue>& issues)
{
    OrderedJson json;
    std::map<std::string, OrderedJson, std::less<>> tables;
    std::map<std::string, OrderedJson, std::less<>> profiles;
    for (const auto& [path, value] : fields)
    {
        const std::vector<std::string> parts = SplitPath(path);
        if (parts[0] == DropLevelBonusEntry && parts.size() == 2)
        {
            json[DropLevelBonusEntry][parts[1]] = value;
        }
        else if (parts[0] == TablesEntry && parts.size() == 2)
        {
            OrderedJson table;
            table[NameField] = parts[1];
            for (const auto& [key, entry] : value.items())
            {
                table[key] = entry;
            }
            tables[parts[1]] = std::move(table);
        }
        else if (parts[0] == DefaultEntry && parts.size() == 3)
        {
            json[DefaultEntry][parts[1]][parts[2]] = value;
        }
        else if (parts[0] == ProfilesEntry && (parts.size() == 2 || parts.size() == 4))
        {
            OrderedJson& profile = profiles[parts[1]];
            profile[NameField] = parts[1];
            if (parts.size() == 4)
            {
                profile[parts[2]][parts[3]] = value;
            }
        }
    }
    json[TablesEntry] = OrderedJson::array();
    for (auto& [name, table] : tables)
    {
        json[TablesEntry].push_back(std::move(table));
    }
    json[ProfilesEntry] = OrderedJson::array();
    for (auto& [name, profile] : profiles)
    {
        json[ProfilesEntry].push_back(std::move(profile));
    }

    std::vector<ItemDataIssue> readIssues;
    Data::Items::ItemStatFile file;
    Data::Items::ReadItemStatFormulasJson(json, "formulas", file, readIssues);
    const bool valid = !Data::Items::HasErrors(readIssues);
    issues.insert(issues.end(), readIssues.begin(), readIssues.end());
    return valid ? std::optional(std::move(file)) : std::nullopt;
}

// The profile a path belongs to ("profiles.weapon" for "profiles.weapon.damage.table").
std::optional<std::string> ProfileOf(const std::string& path)
{
    const std::vector<std::string> parts = SplitPath(path);
    if (parts.size() < 2 || parts[0] != ProfilesEntry)
    {
        return std::nullopt;
    }
    return parts[0] + "." + parts[1];
}

std::string PartsOf(const Fields& fields, const std::string& profile)
{
    OrderedJson json = OrderedJson::object();
    for (const auto& [path, value] : fields)
    {
        if (path.rfind(profile + ".", 0) == 0)
        {
            json[path.substr(profile.size() + 1)] = value;
        }
    }
    return ToText(json);
}

void CompareFormulas(const Data::Items::ItemStatFile& current, const Data::Items::ItemStatFile& incoming,
                     const SyncOptions& options, std::vector<SyncFormula>& rows)
{
    const Fields currentFields = FlattenFormulas(current);
    const Fields incomingFields = FlattenFormulas(incoming);
    std::set<std::string, std::less<>> paths;
    for (const auto& [path, value] : currentFields)
    {
        paths.insert(path);
    }
    for (const auto& [path, value] : incomingFields)
    {
        paths.insert(path);
    }
    // A profile one side lacks is one row as a whole.
    std::set<std::string, std::less<>> wholeProfiles;
    for (const std::string& path : paths)
    {
        const std::optional<std::string> profile = ProfileOf(path);
        if (profile && *profile == path && currentFields.contains(path) != incomingFields.contains(path))
        {
            wholeProfiles.insert(path);
        }
    }
    for (const std::string& path : paths)
    {
        const std::optional<std::string> profile = ProfileOf(path);
        SyncFormula row;
        if (profile && wholeProfiles.contains(*profile))
        {
            if (path != *profile)
            {
                continue;
            }
            row.entry = path;
            if (currentFields.contains(path))
            {
                row.current = PartsOf(currentFields, path);
            }
            if (incomingFields.contains(path))
            {
                row.incoming = PartsOf(incomingFields, path);
            }
        }
        else
        {
            row.entry = path;
            row.current = TextOf(currentFields, path);
            row.incoming = TextOf(incomingFields, path);
            if (row.current == row.incoming)
            {
                continue;
            }
        }
        row.known = options.known != nullptr && options.known->Contains(row.entry);
        row.checked = !row.known;
        rows.push_back(std::move(row));
    }
}

void ApplyFormulas(const ItemExchangeFile& source, ItemExchangeFile& target, const ItemSyncDiff& diff,
                   std::vector<ItemDataIssue>& issues)
{
    if (!source.formulas || !target.formulas ||
        std::ranges::none_of(diff.formulas, [](const SyncFormula& row) { return row.checked; }))
    {
        return;
    }
    const Fields from = FlattenFormulas(*source.formulas);
    Fields fields = FlattenFormulas(*target.formulas);
    const auto take = [&](const std::string& path)
    {
        if (const auto found = from.find(path); found != from.end())
        {
            fields[path] = found->second;
        }
        else
        {
            fields.erase(path);
        }
    };
    for (const SyncFormula& row : diff.formulas)
    {
        if (!row.checked)
        {
            continue;
        }
        const std::optional<std::string> profile = ProfileOf(row.entry);
        if (profile && *profile == row.entry)
        {
            // A profile as a whole: its marker and every term.
            std::erase_if(fields, [&](const auto& entry) { return entry.first.rfind(row.entry + ".", 0) == 0; });
            for (const auto& [path, value] : from)
            {
                if (path.rfind(row.entry + ".", 0) == 0)
                {
                    fields[path] = value;
                }
            }
        }
        take(row.entry);
    }
    if (std::optional<Data::Items::ItemStatFile> formulas = UnflattenFormulas(fields, issues))
    {
        target.formulas = std::move(*formulas);
    }
}
} // namespace

bool KnownDifferences::Contains(int group, int number, const std::string& field) const
{
    return fields.contains({group, number, field});
}

bool KnownDifferences::Contains(const std::string& formula) const
{
    return formulas.contains(formula);
}

std::filesystem::path GetKnownDifferencesFile()
{
    return Data::Items::GetItemDataDirectory() / "Sync" / "KnownDifferences.json";
}

KnownDifferences LoadKnownDifferences(const std::filesystem::path& file, std::vector<ItemDataIssue>& issues)
{
    KnownDifferences known;
    const std::optional<std::string> text = Data::Items::Json::ReadTextFile(file);
    if (!text)
    {
        return known;
    }
    const std::string source = file.generic_string();
    OrderedJson root;
    if (!Data::Items::Json::ReadFileVersion(*text, source, KnownDifferencesFormatVersion, root, issues))
    {
        return known;
    }
    const auto addError = [&](const std::string& field, const std::string& message)
    {
        issues.push_back(
            {ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
    };
    if (const auto items = root.find("items"); items != root.end() && items->is_array())
    {
        for (const OrderedJson& item : *items)
        {
            if (!item.is_object() || !item.contains("group") || !item.contains("number") ||
                !item["group"].is_number_integer() || !item["number"].is_number_integer() || !item.contains("fields") ||
                !item["fields"].is_array())
            {
                addError("items", "an entry needs a group, a number and a list of fields");
                continue;
            }
            for (const OrderedJson& field : item["fields"])
            {
                if (field.is_string())
                {
                    known.fields.insert(
                        {item["group"].get<int>(), item["number"].get<int>(), field.get<std::string>()});
                }
            }
        }
    }
    if (const auto formulas = root.find("formulas"); formulas != root.end() && formulas->is_array())
    {
        for (const OrderedJson& formula : *formulas)
        {
            if (formula.is_string())
            {
                known.formulas.insert(formula.get<std::string>());
            }
        }
    }
    return known;
}

std::string WriteKnownDifferencesJson(const KnownDifferences& known)
{
    OrderedJson root;
    root["formatVersion"] = KnownDifferencesFormatVersion;
    root["items"] = OrderedJson::array();
    std::map<ItemKey, std::vector<std::string>> byItem;
    for (const auto& [group, number, field] : known.fields)
    {
        byItem[{group, number}].push_back(field);
    }
    for (const auto& [key, fields] : byItem)
    {
        OrderedJson item;
        item["group"] = key.first;
        item["number"] = key.second;
        item["fields"] = fields;
        root["items"].push_back(std::move(item));
    }
    root["formulas"] = OrderedJson::array();
    for (const std::string& formula : known.formulas)
    {
        root["formulas"].push_back(formula);
    }
    const std::string text = root.dump(Data::Items::Json::Indent);
    return Data::Items::Json::PutListEntriesOnOneLine(text, "items") + "\n";
}

ItemSyncDiff BuildItemSyncDiff(const ItemExchangeFile& client, const ItemExchangeFile& file, const SyncOptions& options)
{
    ItemSyncDiff diff;
    diff.direction = options.direction;
    if (options.direction == SyncDirection::Import)
    {
        diff.takenLocales.assign(options.uiLocales.begin(), options.uiLocales.end());
    }
    const ItemExchangeFile& current = options.direction == SyncDirection::Import ? client : file;
    const ItemExchangeFile& incoming = options.direction == SyncDirection::Import ? file : client;
    const std::map<ItemKey, const ItemDefinition*> currentItems = IndexItems(current);
    const std::map<ItemKey, const ItemDefinition*> incomingItems = IndexItems(incoming);
    std::set<ItemKey> keys;
    for (const auto& [key, item] : currentItems)
    {
        keys.insert(key);
    }
    for (const auto& [key, item] : incomingItems)
    {
        keys.insert(key);
    }
    for (const ItemKey& key : keys)
    {
        const auto was = currentItems.find(key);
        const auto becomes = incomingItems.find(key);
        SyncItem row = CompareItem(key, was != currentItems.end() ? was->second : nullptr,
                                   becomes != incomingItems.end() ? becomes->second : nullptr, incoming, options);
        if (row.change != SyncChange::Changed || !row.fields.empty())
        {
            diff.items.push_back(std::move(row));
        }
    }
    if (current.formulas && incoming.formulas)
    {
        CompareFormulas(*current.formulas, *incoming.formulas, options, diff.formulas);
    }
    return diff;
}

ItemExchangeFile ApplyItemSyncDiff(const ItemExchangeFile& client, const ItemExchangeFile& file,
                                   const ItemSyncDiff& diff, std::vector<ItemDataIssue>& issues)
{
    const bool import = diff.direction == SyncDirection::Import;
    ItemExchangeFile target = import ? client : file;
    const ItemExchangeFile& source = import ? file : client;
    const std::map<ItemKey, const ItemDefinition*> sourceItems = IndexItems(source);
    std::map<ItemKey, ItemDefinition> targetItems;
    for (const ItemDefinition& item : target.items)
    {
        targetItems[{item.group, item.number}] = item;
    }
    for (const SyncItem& row : diff.items)
    {
        const ItemKey key{row.group, row.number};
        const auto from = sourceItems.find(key);
        if (row.change == SyncChange::Removed)
        {
            if (row.checked && row.locked.empty())
            {
                targetItems.erase(key);
            }
            continue;
        }
        if (from == sourceItems.end())
        {
            continue;
        }
        const Fields sourceFields = FlattenItem(*from->second);
        if (row.change == SyncChange::New)
        {
            if (!row.checked)
            {
                continue;
            }
            Fields fields;
            for (const SyncField& field : row.fields)
            {
                fields[field.field] = sourceFields.at(field.field);
            }
            targetItems[key] = UnflattenItem(row.group, row.number, fields, issues);
            continue;
        }
        Fields fields = FlattenItem(targetItems.at(key));
        bool changed = false;
        for (const SyncField& field : row.fields)
        {
            if (!field.checked || !field.locked.empty())
            {
                continue;
            }
            if (const auto value = sourceFields.find(field.field); value != sourceFields.end())
            {
                fields[field.field] = value->second;
            }
            else
            {
                fields.erase(field.field);
            }
            changed = true;
        }
        if (changed)
        {
            targetItems[key] = UnflattenItem(row.group, row.number, fields, issues);
        }
    }
    target.items.clear();
    for (auto& [key, item] : targetItems)
    {
        target.items.push_back(std::move(item));
    }
    ApplyFormulas(source, target, diff, issues);
    return target;
}
} // namespace MuEditor::Items

#endif // _EDITOR
