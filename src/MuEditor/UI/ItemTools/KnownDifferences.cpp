#include "stdafx.h"

#ifdef _EDITOR

#include "KnownDifferences.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <algorithm>
#include <optional>
#include <utility>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Items::Json::OrderedJson;
using ItemKey = std::pair<int, int>;

// Version 2 names the wrong side of each entry; version 1 listed "known" differences, which kept
// the client's value.
constexpr int KnownDifferencesFormatVersion = 2;
constexpr int KnownListFormatVersion = 1;
constexpr const char* FormatVersionKey = "formatVersion";
constexpr const char* ItemsKey = "items";
constexpr const char* FormulasKey = "formulas";
constexpr const char* GroupKey = "group";
constexpr const char* NumberKey = "number";
// Version 1's list of an item's known fields.
constexpr const char* KnownFieldsKey = "fields";

struct SideKey
{
    WrongSide side;
    const char* key;
};

constexpr SideKey SideKeys[] = {
    {WrongSide::Client, "wrongInClient"},
    {WrongSide::Server, "wrongInServer"},
};

// The list a side is written to; nothing for WrongSide::None, which is no mark.
const char* KeyOf(WrongSide side)
{
    for (const auto& [listed, key] : SideKeys)
    {
        if (listed == side)
        {
            return key;
        }
    }
    return nullptr;
}

// Whether `list` is a list of names. A list with something else in it is not read at all, so it
// is reported rather than written back without that entry.
bool IsNameList(const OrderedJson& list)
{
    return list.is_array() && std::ranges::all_of(list, [](const OrderedJson& entry) { return entry.is_string(); });
}

// The names of a list IsNameList accepted.
std::vector<std::string> ReadStrings(const OrderedJson& list)
{
    std::vector<std::string> strings;
    for (const OrderedJson& entry : list)
    {
        strings.push_back(entry.get<std::string>());
    }
    return strings;
}

// Whether `entry` has a side's list (and every side it has is a list of names).
bool HasSideLists(const OrderedJson& entry)
{
    bool any = false;
    for (const auto& [side, key] : SideKeys)
    {
        if (const auto list = entry.find(key); list != entry.end())
        {
            if (!IsNameList(*list))
            {
                return false;
            }
            any = true;
        }
    }
    return any;
}

bool IsValidItem(const OrderedJson& item, bool knownList)
{
    const bool numbered = item.is_object() && item.contains(GroupKey) && item.contains(NumberKey) &&
                          item[GroupKey].is_number_integer() && item[NumberKey].is_number_integer();
    if (!numbered)
    {
        return false;
    }
    return knownList ? item.contains(KnownFieldsKey) && IsNameList(item[KnownFieldsKey]) : HasSideLists(item);
}

// Version 1: a list; version 2: an object with the sides' lists (an empty one is fine).
bool IsValidFormulas(const OrderedJson& formulas, bool knownList)
{
    if (knownList)
    {
        return IsNameList(formulas);
    }
    return formulas.is_object() &&
           std::ranges::all_of(SideKeys, [&](const SideKey& side)
                               { return !formulas.contains(side.key) || IsNameList(formulas[side.key]); });
}

void ReadItem(const OrderedJson& item, bool knownList, KnownDifferences& known)
{
    const int group = item[GroupKey].get<int>();
    const int number = item[NumberKey].get<int>();
    if (knownList)
    {
        for (const std::string& field : ReadStrings(item[KnownFieldsKey]))
        {
            known.Set(group, number, field, WrongSide::Server);
        }
        return;
    }
    for (const auto& [side, key] : SideKeys)
    {
        if (const auto fields = item.find(key); fields != item.end())
        {
            for (const std::string& field : ReadStrings(*fields))
            {
                known.Set(group, number, field, side);
            }
        }
    }
}

void ReadFormulas(const OrderedJson& formulas, bool knownList, KnownDifferences& known)
{
    if (knownList)
    {
        for (const std::string& formula : ReadStrings(formulas))
        {
            known.Set(formula, WrongSide::Server);
        }
        return;
    }
    for (const auto& [side, key] : SideKeys)
    {
        if (const auto entries = formulas.find(key); entries != formulas.end())
        {
            for (const std::string& formula : ReadStrings(*entries))
            {
                known.Set(formula, side);
            }
        }
    }
}

OrderedJson WriteItems(const KnownDifferences& known)
{
    std::map<ItemKey, std::map<WrongSide, std::vector<std::string>>> byItem;
    for (const auto& [entry, side] : known.fields)
    {
        if (KeyOf(side) == nullptr)
        {
            continue;
        }
        const auto& [group, number, field] = entry;
        byItem[{group, number}][side].push_back(field);
    }
    OrderedJson items = OrderedJson::array();
    for (const auto& [key, sides] : byItem)
    {
        OrderedJson item;
        item[GroupKey] = key.first;
        item[NumberKey] = key.second;
        for (const auto& [side, sideKey] : SideKeys)
        {
            if (const auto fields = sides.find(side); fields != sides.end())
            {
                item[sideKey] = fields->second;
            }
        }
        items.push_back(std::move(item));
    }
    return items;
}

OrderedJson WriteFormulas(const KnownDifferences& known)
{
    OrderedJson formulas = OrderedJson::object();
    for (const auto& [side, key] : SideKeys)
    {
        formulas[key] = OrderedJson::array();
    }
    for (const auto& [formula, side] : known.formulas)
    {
        if (const char* key = KeyOf(side))
        {
            formulas[key].push_back(formula);
        }
    }
    return formulas;
}
} // namespace

WrongSide KnownDifferences::Find(int group, int number, const std::string& field) const
{
    const auto entry = fields.find({group, number, field});
    return entry != fields.end() ? entry->second : WrongSide::None;
}

WrongSide KnownDifferences::Find(const std::string& formula) const
{
    const auto entry = formulas.find(formula);
    return entry != formulas.end() ? entry->second : WrongSide::None;
}

void KnownDifferences::Set(int group, int number, const std::string& field, WrongSide side)
{
    if (side == WrongSide::None)
    {
        fields.erase({group, number, field});
        return;
    }
    fields[{group, number, field}] = side;
}

bool KnownDifferences::ClearChangedItem(int group, int number)
{
    bool cleared = false;
    // The item's entries are in a row, from the empty field on.
    auto entry = fields.lower_bound({group, number, std::string()});
    while (entry != fields.end() && std::get<0>(entry->first) == group && std::get<1>(entry->first) == number)
    {
        if (std::get<2>(entry->first) == AddedOrRemovedItemField)
        {
            ++entry;
            continue;
        }
        entry = fields.erase(entry);
        cleared = true;
    }
    return cleared;
}

void KnownDifferences::Set(const std::string& formula, WrongSide side)
{
    if (side == WrongSide::None)
    {
        formulas.erase(formula);
        return;
    }
    formulas[formula] = side;
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
    const bool knownList = root[FormatVersionKey].get<int>() == KnownListFormatVersion;
    // What is not read is reported: the next save would leave it out.
    const auto addError = [&](const char* field, const char* message)
    {
        issues.push_back(
            {ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
    };
    if (const auto items = root.find(ItemsKey); items != root.end())
    {
        if (!items->is_array())
        {
            addError(ItemsKey, "the items need to be a list");
        }
        else
        {
            for (const OrderedJson& item : *items)
            {
                if (!IsValidItem(item, knownList))
                {
                    addError(ItemsKey, knownList ? "an entry needs a group, a number and a list of field names"
                                                 : "an entry needs a group, a number and a wrongInClient or "
                                                   "wrongInServer list of field names");
                    continue;
                }
                ReadItem(item, knownList, known);
            }
        }
    }
    if (const auto formulas = root.find(FormulasKey); formulas != root.end())
    {
        if (!IsValidFormulas(*formulas, knownList))
        {
            addError(FormulasKey, knownList ? "the formulas need to be a list of names"
                                            : "the formulas need to be an object whose wrongInClient and "
                                              "wrongInServer, where given, are lists of names");
        }
        else
        {
            ReadFormulas(*formulas, knownList, known);
        }
    }
    return known;
}

std::string WriteKnownDifferencesJson(const KnownDifferences& known)
{
    OrderedJson root;
    root[FormatVersionKey] = KnownDifferencesFormatVersion;
    root[ItemsKey] = WriteItems(known);
    root[FormulasKey] = WriteFormulas(known);
    const std::string text = root.dump(Data::Items::Json::Indent);
    return Data::Items::Json::PutListEntriesOnOneLine(text, ItemsKey) + "\n";
}
} // namespace MuEditor::Items

#endif // _EDITOR
