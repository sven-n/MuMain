#include "stdafx.h"

#ifdef _EDITOR

#include "ItemSyncDiff.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemStatsJson.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"

#include <algorithm>
#include <map>
#include <utility>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDefinition;
using Data::Items::ItemExchangeFile;
using Data::Items::Json::OrderedJson;

// A side's fields by their path, as JSON.
using Fields = std::map<std::string, OrderedJson, std::less<>>;
using ItemKey = std::pair<int, int>;

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
// and an import takes names only in the client's UI languages. A comparison of two files
// (comparingFiles) takes every one.
bool TakesPart(const std::string& path, bool incomingHasIt, const SyncOptions& options)
{
    std::string locale;
    if (!IsNamePath(path, locale) || options.comparingFiles)
    {
        return true;
    }
    if (!incomingHasIt)
    {
        return false;
    }
    return std::ranges::find(options.uiLocales, locale) != options.uiLocales.end();
}

bool IsHeld(const SyncOptions& options, const ItemKey& key)
{
    return options.isHeld && options.isHeld(Data::Items::MakeItemType(key.first, key.second));
}

void MarkField(SyncField& field, const ItemKey& key, const ItemExchangeFile& incoming, const Fields& current,
               const Fields& incomingFields, const SyncOptions& options)
{
    field.known = options.known != nullptr && options.known->Contains(key.first, key.second, field.field);
    if (IsHeld(options, key) && std::ranges::find(HeldFields, field.field) != std::end(HeldFields))
    {
        field.locked = I18N::Editor::TheItemIsWornOrInAnItemWindowItsSizeAndSlotStayAsTheyAre;
    }
    if (field.field == LevelField && incoming.source == Data::Items::ItemExchangeSourceOpenMU)
    {
        const auto client = current.find(LevelField);
        const auto server = incomingFields.find(LevelField);
        if (client != current.end() && client->second.get<int>() > OpenMUMaxDropLevel &&
            server != incomingFields.end() && server->second.get<int>() == OpenMUMaxDropLevel)
        {
            field.inexact = I18N::Editor::OpenMUKeepsTheDropLevelInAByteAtMost255;
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
        // An item of a type the client has no data for yet can be held too (the server sent
        // it): its size would change under the item grid (ItemEdits refuses it).
        if (IsHeld(options, key))
        {
            row.locked = I18N::Editor::TheItemIsWornOrInAnItemWindowItIsNotAdded;
        }
        row.checked = !row.known && row.locked.empty();
        return row;
    }
    if (incoming == nullptr)
    {
        row.change = SyncChange::Removed;
        // A comparison of two files shows what file A has (nothing applies them).
        if (options.comparingFiles)
        {
            for (const auto& [path, value] : currentFields)
            {
                row.fields.push_back({path, ToText(value), std::nullopt});
            }
        }
        if (IsHeld(options, key))
        {
            row.locked = I18N::Editor::TheItemIsWornOrInAnItemWindowItIsNotMovedOrRemoved;
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
                     const SyncOptions& options, ItemSyncDiff& diff)
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
            ++diff.comparedFormulaEntries;
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
            // The entry of a profile both sides have only says that it is there.
            const bool isEntry = !profile || *profile != path;
            row.entry = path;
            row.current = TextOf(currentFields, path);
            row.incoming = TextOf(incomingFields, path);
            diff.comparedFormulaEntries += isEntry ? 1 : 0;
            if (row.current == row.incoming)
            {
                diff.sameFormulaEntries += isEntry ? 1 : 0;
                continue;
            }
        }
        row.known = options.known != nullptr && options.known->Contains(row.entry);
        // An entry the file lacks starts unchecked, as a removed item does: taking it away would
        // leave the items that name a profile or table without it.
        row.checked = !row.known && row.incoming.has_value();
        diff.formulas.push_back(std::move(row));
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

ItemSyncDiff BuildItemSyncDiff(const ItemExchangeFile& client, const ItemExchangeFile& file, const SyncOptions& options)
{
    ItemSyncDiff diff;
    const ItemExchangeFile& current = client;
    const ItemExchangeFile& incoming = file;
    const Data::Items::ExchangeItemIndex currentItems = Data::Items::IndexExchangeItems(current);
    const Data::Items::ExchangeItemIndex incomingItems = Data::Items::IndexExchangeItems(incoming);
    // An item a side has more than once (a file read with repeatedItems; the client's data never
    // has one): an error row that is not applied.
    const std::set<ItemKey> currentRepeated(client.repeatedItems.begin(), client.repeatedItems.end());
    const std::set<ItemKey> incomingRepeated(file.repeatedItems.begin(), file.repeatedItems.end());
    std::set<ItemKey> repeated = currentRepeated;
    repeated.insert(incomingRepeated.begin(), incomingRepeated.end());
    std::set<ItemKey> keys = repeated;
    for (const auto& [key, item] : currentItems)
    {
        keys.insert(key);
    }
    for (const auto& [key, item] : incomingItems)
    {
        keys.insert(key);
    }
    diff.comparedItems = static_cast<int>(keys.size());
    for (const ItemKey& key : keys)
    {
        const auto was = currentItems.find(key);
        const auto becomes = incomingItems.find(key);
        if (repeated.contains(key))
        {
            // A side that has the item twice has it too, though not among its items.
            const bool inCurrent = was != currentItems.end() || currentRepeated.contains(key);
            const bool inIncoming = becomes != incomingItems.end() || incomingRepeated.contains(key);
            SyncItem row;
            row.group = key.first;
            row.number = key.second;
            row.change = !inCurrent ? SyncChange::New : !inIncoming ? SyncChange::Removed : SyncChange::Changed;
            if (becomes != incomingItems.end())
            {
                row.name = becomes->second->names.GetNeutral();
            }
            else if (was != currentItems.end())
            {
                row.name = was->second->names.GetNeutral();
            }
            if (!options.comparingFiles)
            {
                row.locked = I18N::Editor::ItIsInTheFileMoreThanOnce;
            }
            else
            {
                row.locked = currentRepeated.contains(key) ? I18N::Editor::ItIsInFileAMoreThanOnce
                                                           : I18N::Editor::ItIsInFileBMoreThanOnce;
            }
            diff.items.push_back(std::move(row));
            continue;
        }
        SyncItem row = CompareItem(key, was != currentItems.end() ? was->second : nullptr,
                                   becomes != incomingItems.end() ? becomes->second : nullptr, incoming, options);
        if (row.change != SyncChange::Changed || !row.fields.empty())
        {
            diff.items.push_back(std::move(row));
        }
        else
        {
            ++diff.sameItems;
        }
    }
    if (current.formulas && incoming.formulas)
    {
        CompareFormulas(*current.formulas, *incoming.formulas, options, diff);
    }
    return diff;
}

int CountFormulaDifferences(const Data::Items::ItemStatFile& current, const Data::Items::ItemStatFile& incoming)
{
    ItemSyncDiff diff;
    CompareFormulas(current, incoming, SyncOptions{}, diff);
    return static_cast<int>(diff.formulas.size());
}

ItemExchangeFile ApplyItemSyncDiff(const ItemExchangeFile& client, const ItemExchangeFile& file,
                                   const ItemSyncDiff& diff, std::vector<ItemDataIssue>& issues)
{
    ItemExchangeFile target = client;
    const ItemExchangeFile& source = file;
    const Data::Items::ExchangeItemIndex sourceItems = Data::Items::IndexExchangeItems(source);
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
            if (!row.checked || !row.locked.empty())
            {
                continue;
            }
            Fields fields;
            for (const SyncField& field : row.fields)
            {
                // A field the source no longer has (its data changed since the comparison) keeps
                // its default.
                if (const auto value = sourceFields.find(field.field); value != sourceFields.end())
                {
                    fields[field.field] = value->second;
                }
            }
            targetItems[key] = UnflattenItem(row.group, row.number, fields, issues);
            continue;
        }
        // An item the client no longer has (its data changed since the diff) is left out.
        const auto target = targetItems.find(key);
        if (target == targetItems.end())
        {
            continue;
        }
        Fields fields = FlattenItem(target->second);
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
