#include "stdafx.h"

#ifdef _EDITOR

#include "ItemExchangeFile.h"

#include "ItemJsonCommon.h"
#include "ItemJsonFormat.h"
#include "ItemStatProfiles.h"
#include "ItemStatsJson.h"
#include "ItemType.h"

#include <algorithm>
#include <map>
#include <utility>

namespace Data::Items
{
namespace
{
using Json::OrderedJson;

namespace Keys
{
constexpr const char* FormatVersion = "formatVersion";
constexpr const char* Source = "source";
constexpr const char* Server = "server";
constexpr const char* GameVersion = "gameVersion";
constexpr const char* Formulas = "formulas";
constexpr const char* Items = "items";
constexpr const char* Group = "group";
} // namespace Keys

const Json::KeySet RootKeys = {Keys::FormatVersion, Keys::Source,   Keys::Server,
                               Keys::GameVersion,   Keys::Formulas, Keys::Items};

bool LessByKey(const ItemDefinition& left, const ItemDefinition& right)
{
    return std::pair(left.group, left.number) < std::pair(right.group, right.number);
}

class ExchangeReader
{
public:
    ExchangeReader(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues)
    {
    }

    void Read(const OrderedJson& root, ItemExchangeFile& file);

private:
    void AddIssue(ItemDataIssueSeverity severity, int group, int number, const std::string& field,
                  const std::string& message)
    {
        m_issues.push_back({severity, m_source, group, number, field, message});
    }

    // A text of `root`; false when it is missing (and required) or not a text.
    bool ReadText(const OrderedJson& root, const char* key, bool required, std::string& value);
    void ReadItems(const OrderedJson& root, ItemExchangeFile& file);
    void ReadItem(const OrderedJson& json, size_t index, std::vector<ItemDefinition>& items);

    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

bool ExchangeReader::ReadText(const OrderedJson& root, const char* key, bool required, std::string& value)
{
    const auto field = root.find(key);
    if (field == root.end())
    {
        if (required)
        {
            AddIssue(ItemDataIssueSeverity::Error, ItemDataIssue::NoItem, ItemDataIssue::NoItem, key, "missing");
        }
        return false;
    }
    if (!field->is_string())
    {
        AddIssue(ItemDataIssueSeverity::Error, ItemDataIssue::NoItem, ItemDataIssue::NoItem, key, "must be a text");
        return false;
    }
    if (field->get_ref<const std::string&>().empty())
    {
        // An empty optional text is as if it were not there (the writer leaves it out).
        if (required)
        {
            AddIssue(ItemDataIssueSeverity::Error, ItemDataIssue::NoItem, ItemDataIssue::NoItem, key, "must be a text");
        }
        return false;
    }
    value = field->get<std::string>();
    return true;
}

void ExchangeReader::Read(const OrderedJson& root, ItemExchangeFile& file)
{
    Json::WarnAboutUnknownKeys(
        root, RootKeys, "", [this](const std::string& field, const std::string& message)
        { AddIssue(ItemDataIssueSeverity::Warning, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message); });

    std::string text;
    if (ReadText(root, Keys::Source, true, text))
    {
        if (text != ItemExchangeSourceMuMain && text != ItemExchangeSourceOpenMU)
        {
            AddIssue(ItemDataIssueSeverity::Error, ItemDataIssue::NoItem, ItemDataIssue::NoItem, Keys::Source,
                     std::string("must be \"") + ItemExchangeSourceMuMain + "\" or \"" + ItemExchangeSourceOpenMU +
                         "\"");
        }
        file.source = text;
    }
    ReadText(root, Keys::Server, false, file.server);
    if (ReadText(root, Keys::GameVersion, true, text))
    {
        if (text != ItemExchangeGameVersion)
        {
            AddIssue(ItemDataIssueSeverity::Error, ItemDataIssue::NoItem, ItemDataIssue::NoItem, Keys::GameVersion,
                     std::string("only \"") + ItemExchangeGameVersion + "\" data can be exchanged");
        }
        file.gameVersion = text;
    }

    if (const auto formulas = root.find(Keys::Formulas); formulas != root.end())
    {
        ItemStatFile statFile;
        ReadItemStatFormulasJson(*formulas, m_source, statFile, m_issues);
        file.formulas = std::move(statFile);
    }
    ReadItems(root, file);
}

void ExchangeReader::ReadItems(const OrderedJson& root, ItemExchangeFile& file)
{
    const auto list = root.find(Keys::Items);
    if (list == root.end() || !list->is_array())
    {
        AddIssue(ItemDataIssueSeverity::Error, ItemDataIssue::NoItem, ItemDataIssue::NoItem, Keys::Items,
                 "missing or not a list");
        return;
    }
    std::vector<ItemDefinition> items;
    for (size_t index = 0; index < list->size(); ++index)
    {
        ReadItem((*list)[index], index, items);
    }

    // A (group, number) that is there twice (OpenMU does not keep it unique): neither is read.
    std::map<std::pair<int, int>, int> counts;
    for (const ItemDefinition& item : items)
    {
        ++counts[{item.group, item.number}];
    }
    for (const auto& [key, count] : counts)
    {
        if (count > 1)
        {
            AddIssue(ItemDataIssueSeverity::Error, key.first, key.second, "",
                     "is in the file " + std::to_string(count) + " times; none of them is read");
            file.repeatedItems.push_back(key);
        }
    }
    std::erase_if(items, [&](const ItemDefinition& item) { return counts[{item.group, item.number}] > 1; });
    std::ranges::sort(items, LessByKey);
    file.items = std::move(items);
}

void ExchangeReader::ReadItem(const OrderedJson& json, size_t index, std::vector<ItemDefinition>& items)
{
    const std::string field = std::string(Keys::Items) + "[" + std::to_string(index) + "]";
    if (!json.is_object())
    {
        AddIssue(ItemDataIssueSeverity::Error, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field,
                 "an item must be an object");
        return;
    }
    const auto groupField = json.find(Keys::Group);
    long long group = 0;
    if (groupField == json.end() || !groupField->is_number_integer() || !Json::ReadWholeNumber(*groupField, group) ||
        group < 0 || group >= MAX_ITEM_TYPE)
    {
        AddIssue(ItemDataIssueSeverity::Error, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field + "." + Keys::Group,
                 "missing or not a group from 0 to " + std::to_string(MAX_ITEM_TYPE - 1));
        return;
    }

    // The rest is an item of the item files.
    OrderedJson itemJson = json;
    itemJson.erase(Keys::Group);
    ItemDefinition item;
    if (!ReadItemJson(itemJson, m_source, static_cast<int>(group), item, m_issues))
    {
        return;
    }
    ItemDefinition shared = ToExchangeItem(item);
    if (item.statProfile.empty())
    {
        item.statProfile = shared.statProfile;
    }
    if (!(shared == item))
    {
        AddIssue(ItemDataIssueSeverity::Warning, item.group, item.number, "",
                 "has fields the exchange file does not have (they stay with the client); they are left out");
    }
    items.push_back(std::move(shared));
}
} // namespace

ExchangeItemIndex IndexExchangeItems(const ItemExchangeFile& file)
{
    ExchangeItemIndex items;
    for (const ItemDefinition& item : file.items)
    {
        items[{item.group, item.number}] = &item;
    }
    return items;
}

void CopyExchangeFields(const ItemDefinition& from, ItemDefinition& to)
{
    to.names = from.names;
    for (const ItemTag tag : ExchangeTags)
    {
        to.tags.Set(tag, from.tags.Has(tag));
    }
    to.width = from.width;
    to.height = from.height;
    to.slot = from.slot;
    to.skill = from.skill;
    to.level = from.level;
    to.durability = from.durability;
    to.magicDurability = from.magicDurability;
    to.damageMin = from.damageMin;
    to.damageMax = from.damageMax;
    to.blockRate = from.blockRate;
    to.defense = from.defense;
    to.attackSpeed = from.attackSpeed;
    to.walkSpeed = from.walkSpeed;
    to.magicPower = from.magicPower;
    to.excellentBonus = from.excellentBonus;
    to.excellentRaisesLevel = from.excellentRaisesLevel;
    to.requirements = from.requirements;
    to.requirementsByLevel = from.requirementsByLevel;
    to.classRequirements = from.classRequirements;
    to.resistances = from.resistances;
    to.sellValue = from.sellValue;
    to.buyPrice = from.buyPrice;
    to.tradable = from.tradable;
    to.droppable = from.droppable;
    to.storable = from.storable;
    to.sellable = from.sellable;
    to.personalShopSellable = from.personalShopSellable;
    to.repairable = from.repairable;
}

ItemDefinition ToExchangeItem(const ItemDefinition& item)
{
    ItemDefinition shared;
    shared.group = item.group;
    shared.number = item.number;
    CopyExchangeFields(item, shared);
    shared.statProfile = item.statProfile.empty() ? std::string(StatProfileNames::Default) : item.statProfile;
    return shared;
}

ItemExchangeFile BuildItemExchange(std::span<const ItemDefinition> items, const ItemStatFile& formulas)
{
    ItemExchangeFile file;
    file.formulas = formulas;
    file.formulas->profileRules.clear();
    for (const ItemDefinition& item : items)
    {
        if (item.Exists())
        {
            file.items.push_back(ToExchangeItem(item));
        }
    }
    std::ranges::sort(file.items, LessByKey);
    return file;
}

void ReadItemExchangeJson(std::string_view text, const std::string& source, ItemExchangeFile& file,
                          std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, ItemExchangeFormatVersion, root, issues))
    {
        return;
    }
    ExchangeReader(source, issues).Read(root, file);
}

std::string WriteItemExchangeJson(const ItemExchangeFile& file)
{
    OrderedJson root;
    root[Keys::FormatVersion] = ItemExchangeFormatVersion;
    root[Keys::Source] = file.source;
    if (!file.server.empty())
    {
        root[Keys::Server] = file.server;
    }
    root[Keys::GameVersion] = file.gameVersion;
    if (file.formulas)
    {
        root[Keys::Formulas] = WriteItemStatFormulasJson(*file.formulas);
    }

    std::vector<const ItemDefinition*> items;
    for (const ItemDefinition& item : file.items)
    {
        items.push_back(&item);
    }
    std::ranges::sort(items,
                      [](const ItemDefinition* left, const ItemDefinition* right) { return LessByKey(*left, *right); });
    root[Keys::Items] = OrderedJson::array();
    for (const ItemDefinition* item : items)
    {
        OrderedJson json;
        json[Keys::Group] = item->group;
        OrderedJson fields = WriteItemJson(*item);
        for (auto& [key, value] : fields.items())
        {
            json[key] = std::move(value);
        }
        root[Keys::Items].push_back(std::move(json));
    }

    // Names are UTF-8 already; anything that is not is replaced, as in the item files.
    const std::string text = root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    return LayOutItemStatsText(LayOutItemJsonText(text)) + "\n";
}

bool CanImportItemExchange(const ItemExchangeFile& file, std::span<const ItemDataIssue> issues)
{
    return std::ranges::none_of(issues,
                                [&](const ItemDataIssue& issue)
                                {
                                    const std::pair<int, int> key{issue.group, issue.number};
                                    return issue.severity == ItemDataIssueSeverity::Error &&
                                           std::ranges::find(file.repeatedItems, key) == file.repeatedItems.end();
                                });
}
} // namespace Data::Items

#endif // _EDITOR
