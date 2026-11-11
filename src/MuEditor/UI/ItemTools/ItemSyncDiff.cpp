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
    // The client's entry without a name has none until the file gives it one.
    Data::Items::ReadItemJson(json, "", group, item, issues, false);
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

// Whether both sides carry the field of `path` ("requirements.strength" belongs to
// "requirements"): a file with a "fields" list carries only those, the client's data all.
bool BothCarry(const std::string& path, const ItemExchangeFile& current, const ItemExchangeFile& incoming)
{
    const std::string_view key = std::string_view(path).substr(0, path.find('.'));
    return Data::Items::CarriesField(current, key) && Data::Items::CarriesField(incoming, key);
}

// The English name of an item for its row: the file's, unless it does not carry the names.
std::string RowName(const ItemDefinition* current, const ItemDefinition* incoming)
{
    if (incoming != nullptr && !incoming->names.GetNeutral().empty())
    {
        return incoming->names.GetNeutral();
    }
    return current != nullptr ? current->names.GetNeutral() : std::string();
}

bool IsHeld(const SyncOptions& options, const ItemKey& key)
{
    return options.isHeld && options.isHeld(Data::Items::MakeItemType(key.first, key.second));
}

// Whether a field, an item or a formula entry starts checked: one wrong in the server not,
// otherwise `byDefault` (wrong in the client too: checking it is the user's); a locked one never.
bool StartsChecked(WrongSide side, bool byDefault, bool locked)
{
    if (locked)
    {
        return false;
    }
    return side == WrongSide::Server ? false : byDefault;
}

// A click on a mark: Wrong in server unchecks the box, and leaving it (cleared or switched to
// Wrong in client) gives the box its default back (`start`), as a new comparison would start it.
// Otherwise Wrong in client leaves the box as it is.
void UpdateBoxForMark(bool& checked, WrongSide old, WrongSide side, bool start)
{
    if (side == WrongSide::Server || old == WrongSide::Server)
    {
        checked = start;
    }
}

// A field: an inexact one starts unchecked like a locked one, also when it is marked wrong in the
// client: the file's value is not the client's (a drop level over 255 that OpenMU keeps as 255).
// Its box takes it still.
bool FieldStartsChecked(const SyncField& field)
{
    return StartsChecked(field.wrongSide, true, !field.locked.empty() || !field.inexact.empty());
}

WrongSide MarkedSide(const SyncOptions& options, const ItemKey& key, const std::string& field)
{
    return options.known != nullptr ? options.known->Find(key.first, key.second, field) : WrongSide::None;
}

// The field of the mark of a row's item as a whole.
const std::string& WholeItemField(SyncChange change)
{
    return change == SyncChange::Changed ? EveryFieldOfItem : AddedOrRemovedItemField;
}

// A field's own mark, else the item's (`itemSide`).
void MarkField(SyncField& field, const ItemKey& key, WrongSide itemSide, const ItemExchangeFile& incoming,
               const Fields& current, const Fields& incomingFields, const SyncOptions& options)
{
    field.wrongSide = MarkedSide(options, key, field.field);
    if (field.wrongSide == WrongSide::None)
    {
        field.wrongSide = itemSide;
        field.markOfItem = itemSide != WrongSide::None;
    }
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
    field.checked = FieldStartsChecked(field);
}

SyncItem CompareItem(const ItemKey& key, const ItemDefinition* current, const ItemDefinition* incoming,
                     const ItemExchangeFile& currentFile, const ItemExchangeFile& incomingFile,
                     const SyncOptions& options)
{
    SyncItem row;
    row.group = key.first;
    row.number = key.second;
    row.name = RowName(current, incoming);
    const Fields currentFields = current != nullptr ? FlattenItem(*current) : Fields{};
    const Fields incomingFields = incoming != nullptr ? FlattenItem(*incoming) : Fields{};

    if (current == nullptr)
    {
        row.change = SyncChange::New;
        row.wrongSide = MarkedSide(options, key, AddedOrRemovedItemField);
        for (const auto& [path, value] : incomingFields)
        {
            if (TakesPart(path, true, options) && BothCarry(path, currentFile, incomingFile))
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
        // The fields a file does not carry would get their defaults.
        else if (!options.comparingFiles && !Data::Items::CarriesEveryField(incomingFile))
        {
            row.locked = I18N::Editor::TheFileDoesNotCarryAllFieldsOfAnItemItIsNotAdded;
        }
        row.checked = StartsChecked(row.wrongSide, true, !row.locked.empty());
        return row;
    }
    if (incoming == nullptr)
    {
        row.change = SyncChange::Removed;
        row.wrongSide = MarkedSide(options, key, AddedOrRemovedItemField);
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
        row.checked = StartsChecked(row.wrongSide, false, !row.locked.empty());
        return row;
    }

    row.change = SyncChange::Changed;
    row.wrongSide = MarkedSide(options, key, EveryFieldOfItem);
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
        if (was == becomes || !TakesPart(path, becomes.has_value(), options) ||
            !BothCarry(path, currentFile, incomingFile))
        {
            continue;
        }
        SyncField field{path, was, becomes};
        MarkField(field, key, row.wrongSide, incomingFile, currentFields, incomingFields, options);
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
    const bool valid = !Data::Items::HasFatal(readIssues);
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
        row.wrongSide = options.known != nullptr ? options.known->Find(row.entry) : WrongSide::None;
        // An entry the file lacks starts unchecked, as a removed item does: taking it away would
        // leave the items that name a profile or table without it.
        row.checked = StartsChecked(row.wrongSide, row.incoming.has_value(), false);
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

// The columns of WriteSyncRowsMarkdown's table; the change column's head is the caller's.
constexpr const char* MarkdownItemColumns = "Group | Number | Item";
constexpr const char* MarkdownFieldsColumn = "Fields";
constexpr const char* MarkdownWrongInColumn = "Wrong in";
constexpr const char* MarkdownCellSeparator = " | ";
constexpr const char* MarkdownSeparatorCell = "---|";
// The item's group, number and name (MarkdownItemColumns).
constexpr int MarkdownItemColumnCount = 3;
// The side marked wrong.
constexpr int MarkdownWrongInColumnCount = 1;
// The change and the field count.
constexpr int MarkdownChangeColumnCount = 2;

// A Markdown table cell: a | would end it early, a line break the row.
std::string MarkdownCell(std::string_view text)
{
    std::string cell;
    for (const char c : text)
    {
        if (c == '|')
        {
            cell += "\\|";
        }
        else
        {
            cell += c == '\n' || c == '\r' ? ' ' : c;
        }
    }
    return cell;
}

// What the row does in the client, in the English of the Markdown table.
const char* MarkdownChange(SyncChange change)
{
    switch (change)
    {
    case SyncChange::New:
        return "Added";
    case SyncChange::Removed:
        return "Removed";
    default:
        return "Changed";
    }
}

// The side marked wrong, in the English of the Markdown table; empty when none is.
const char* MarkdownWrongIn(WrongSide side)
{
    switch (side)
    {
    case WrongSide::Client:
        return "Client";
    case WrongSide::Server:
        return "Server";
    default:
        return "";
    }
}

std::string WriteMarkdownRow(const SyncItem& row, bool withChange, bool withWrongIn)
{
    std::string text = "| ";
    if (withChange)
    {
        text += MarkdownChange(row.change);
        text += MarkdownCellSeparator;
    }
    text += std::to_string(row.group) + MarkdownCellSeparator + std::to_string(row.number) + MarkdownCellSeparator +
            MarkdownCell(row.name);
    if (withChange)
    {
        text += MarkdownCellSeparator + std::to_string(row.fields.size());
    }
    if (withWrongIn)
    {
        text += MarkdownCellSeparator;
        text += MarkdownWrongIn(GetRowWrongSide(row));
    }
    return text + " |\n";
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
            row.name = RowName(was != currentItems.end() ? was->second : nullptr,
                               becomes != incomingItems.end() ? becomes->second : nullptr);
            // Its mark shows (and can be cleared); a locked row stays unchecked.
            row.wrongSide = MarkedSide(options, key, WholeItemField(row.change));
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
        SyncItem row =
            CompareItem(key, was != currentItems.end() ? was->second : nullptr,
                        becomes != incomingItems.end() ? becomes->second : nullptr, current, incoming, options);
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

bool IsOnlyInClient(const SyncItem& row)
{
    return row.change == SyncChange::Removed;
}

void SetRowChecked(SyncItem& row, bool checked)
{
    if (row.change != SyncChange::Changed)
    {
        row.checked = checked && row.locked.empty();
        return;
    }
    for (SyncField& field : row.fields)
    {
        field.checked = checked && field.locked.empty();
    }
}

void SetAllRowsChecked(ItemSyncDiff& diff, bool checked)
{
    for (SyncItem& row : diff.items)
    {
        SetRowChecked(row, checked);
    }
    for (SyncFormula& row : diff.formulas)
    {
        row.checked = checked;
    }
}

void SetWrongSide(SyncItem& row, const std::string& field, WrongSide side)
{
    if (field.empty())
    {
        const WrongSide old = row.wrongSide;
        row.wrongSide = side;
        if (row.change != SyncChange::Changed)
        {
            UpdateBoxForMark(row.checked, old, side,
                             StartsChecked(side, row.change == SyncChange::New, !row.locked.empty()));
            return;
        }
        for (SyncField& entry : row.fields)
        {
            const WrongSide oldField = entry.wrongSide;
            entry.wrongSide = side;
            entry.markOfItem = side != WrongSide::None;
            UpdateBoxForMark(entry.checked, oldField, side, FieldStartsChecked(entry));
        }
        return;
    }
    // Without a mark of its own, the field has its item's.
    const WrongSide itemSide = row.change == SyncChange::Changed ? row.wrongSide : WrongSide::None;
    for (SyncField& entry : row.fields)
    {
        if (entry.field == field)
        {
            const WrongSide old = entry.wrongSide;
            entry.wrongSide = side != WrongSide::None ? side : itemSide;
            entry.markOfItem = side == WrongSide::None && itemSide != WrongSide::None;
            UpdateBoxForMark(entry.checked, old, entry.wrongSide, FieldStartsChecked(entry));
        }
    }
}

void SetWrongSide(SyncFormula& row, WrongSide side)
{
    const WrongSide old = row.wrongSide;
    row.wrongSide = side;
    UpdateBoxForMark(row.checked, old, side, StartsChecked(side, row.incoming.has_value(), false));
}

void ApplyMarks(ItemSyncDiff& diff, const KnownDifferences& known)
{
    for (SyncItem& row : diff.items)
    {
        const WrongSide whole = known.Find(row.group, row.number, WholeItemField(row.change));
        if (row.change != SyncChange::Changed)
        {
            if (whole != row.wrongSide)
            {
                SetWrongSide(row, "", whole);
            }
            continue;
        }
        row.wrongSide = whole;
        for (SyncField& field : row.fields)
        {
            const WrongSide own = known.Find(row.group, row.number, field.field);
            const WrongSide old = field.wrongSide;
            field.wrongSide = own != WrongSide::None ? own : whole;
            field.markOfItem = own == WrongSide::None && whole != WrongSide::None;
            if (field.wrongSide != old)
            {
                UpdateBoxForMark(field.checked, old, field.wrongSide, FieldStartsChecked(field));
            }
        }
    }
    for (SyncFormula& row : diff.formulas)
    {
        if (const WrongSide side = known.Find(row.entry); side != row.wrongSide)
        {
            SetWrongSide(row, side);
        }
    }
}

void MarkWrongSide(KnownDifferences& known, SyncItem& row, const std::string& field, WrongSide side)
{
    if (field.empty())
    {
        // The whole item: a changed one's fields' own marks go. The marks of the other state (new
        // or removed, or changed) stay, for when the item is in it again.
        if (row.change == SyncChange::Changed)
        {
            known.ClearChangedItem(row.group, row.number);
        }
        known.Set(row.group, row.number, WholeItemField(row.change), side);
        SetWrongSide(row, "", side);
        return;
    }
    known.Set(row.group, row.number, field, side);
    SetWrongSide(row, field, side);
}

void MarkWrongSide(KnownDifferences& known, SyncFormula& row, WrongSide side)
{
    known.Set(row.entry, side);
    SetWrongSide(row, side);
}

bool ForgetAppliedMarks(const ItemSyncDiff& diff, KnownDifferences& known)
{
    bool changed = false;
    const auto forget = [&](const SyncItem& row, const std::string& field)
    { changed = known.fields.erase({row.group, row.number, field}) > 0 || changed; };
    for (const SyncItem& row : diff.items)
    {
        if (row.change != SyncChange::Changed)
        {
            if (row.checked && row.locked.empty())
            {
                forget(row, AddedOrRemovedItemField);
                // A removed item's other marks go too: its number may come back as another item.
                if (row.change == SyncChange::Removed)
                {
                    changed = known.ClearChangedItem(row.group, row.number) || changed;
                }
            }
            continue;
        }
        bool allApplied = !row.fields.empty();
        for (const SyncField& field : row.fields)
        {
            const bool applied = field.checked && field.locked.empty();
            allApplied = allApplied && applied;
            if (applied)
            {
                forget(row, field.field);
            }
        }
        if (allApplied)
        {
            forget(row, EveryFieldOfItem);
        }
    }
    for (const SyncFormula& row : diff.formulas)
    {
        if (row.checked)
        {
            changed = known.formulas.erase(row.entry) > 0 || changed;
        }
    }
    return changed;
}

WrongSide GetRowWrongSide(const SyncItem& row)
{
    // A changed row's fields have the item's mark unless they have their own (a repeated row has
    // no fields).
    if (row.change != SyncChange::Changed || row.fields.empty())
    {
        return row.wrongSide;
    }
    const WrongSide side = row.fields.front().wrongSide;
    const bool same =
        std::ranges::all_of(row.fields, [side](const SyncField& field) { return field.wrongSide == side; });
    return same ? side : WrongSide::None;
}

std::string WriteSyncRowsMarkdown(std::span<const SyncItem* const> rows, std::string_view title,
                                  std::string_view changeColumn, bool withWrongIn)
{
    const bool withChange = !changeColumn.empty();
    std::string text(title);
    text += "\n\n| ";
    if (withChange)
    {
        text += MarkdownCell(changeColumn) + MarkdownCellSeparator;
    }
    text += MarkdownItemColumns;
    if (withChange)
    {
        text += std::string(MarkdownCellSeparator) + MarkdownFieldsColumn;
    }
    if (withWrongIn)
    {
        text += std::string(MarkdownCellSeparator) + MarkdownWrongInColumn;
    }
    text += " |\n|";
    const int columns = MarkdownItemColumnCount + (withWrongIn ? MarkdownWrongInColumnCount : 0) +
                        (withChange ? MarkdownChangeColumnCount : 0);
    for (int column = 0; column < columns; ++column)
    {
        text += MarkdownSeparatorCell;
    }
    text += "\n";
    for (const SyncItem* row : rows)
    {
        text += WriteMarkdownRow(*row, withChange, withWrongIn);
    }
    return text;
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
