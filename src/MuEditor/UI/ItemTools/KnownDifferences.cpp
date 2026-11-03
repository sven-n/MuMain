#include "stdafx.h"

#ifdef _EDITOR

#include "KnownDifferences.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <map>
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

constexpr int KnownDifferencesFormatVersion = 1;
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
            {ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
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
} // namespace MuEditor::Items

#endif // _EDITOR
