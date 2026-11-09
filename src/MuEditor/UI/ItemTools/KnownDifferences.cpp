#include "stdafx.h"

#ifdef _EDITOR

#include "KnownDifferences.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

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

// The strings of a list; other entries are skipped.
std::vector<std::string> ReadStrings(const OrderedJson& list)
{
    std::vector<std::string> strings;
    if (!list.is_array())
    {
        return strings;
    }
    for (const OrderedJson& entry : list)
    {
        if (entry.is_string())
        {
            strings.push_back(entry.get<std::string>());
        }
    }
    return strings;
}

bool HasItemKey(const OrderedJson& item, bool knownList)
{
    const bool numbered = item.is_object() && item.contains(GroupKey) && item.contains(NumberKey) &&
                          item[GroupKey].is_number_integer() && item[NumberKey].is_number_integer();
    return numbered && (!knownList || (item.contains(KnownFieldsKey) && item[KnownFieldsKey].is_array()));
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
    if (!formulas.is_object())
    {
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
    if (const auto items = root.find(ItemsKey); items != root.end() && items->is_array())
    {
        for (const OrderedJson& item : *items)
        {
            if (!HasItemKey(item, knownList))
            {
                issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                                  ItemsKey,
                                  knownList ? "an entry needs a group, a number and a list of fields"
                                            : "an entry needs a group and a number"});
                continue;
            }
            ReadItem(item, knownList, known);
        }
    }
    if (const auto formulas = root.find(FormulasKey); formulas != root.end())
    {
        ReadFormulas(*formulas, knownList, known);
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
