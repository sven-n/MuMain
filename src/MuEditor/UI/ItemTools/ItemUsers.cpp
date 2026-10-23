#include "stdafx.h"

#ifdef _EDITOR

#include "ItemUsers.h"
#include "ItemStatFileChange.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "Data/GameData/ItemData/ItemType.h"

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDefinition;
using Data::Items::ItemModelDefinition;

template <typename TUses> std::vector<int> FindModelUsers(TUses&& uses)
{
    std::vector<int> users;
    for (const ItemModelDefinition& model : g_ItemModelDatabase.GetAllSlots())
    {
        if (model.Exists() && uses(model))
        {
            users.push_back(Data::Items::MakeItemType(model.group, model.number));
        }
    }
    return users;
}

bool IsDefaultProfile(std::string_view name)
{
    return name.empty() || name == Data::Items::StatProfileNames::Default;
}

// The key of a profile in the counts: "" for the default.
std::string_view CountKey(std::string_view name)
{
    return IsDefaultProfile(name) ? std::string_view() : name;
}

int FindCount(const std::map<std::string, int, std::less<>>& counts, std::string_view name)
{
    const auto found = counts.find(CountKey(name));
    return found != counts.end() ? found->second : 0;
}
} // namespace

std::vector<int> FindSharedModelUsers(std::string_view name)
{
    return FindModelUsers([name](const ItemModelDefinition& model) { return model.model == name; });
}

std::vector<int> FindRenderStyleUsers(std::string_view name)
{
    return FindModelUsers([name](const ItemModelDefinition& model) { return model.renderStyle == name; });
}

std::vector<int> FindItemEffectUsers(std::string_view name)
{
    return FindModelUsers([name](const ItemModelDefinition& model) { return model.itemEffect == name; });
}

std::vector<int> FindGlowColorUsers(std::string_view name)
{
    return FindModelUsers(
        [name](const ItemModelDefinition& model)
        { return model.glow.color == name || model.glow.shineColor == name || model.glow.ancientColor == name; });
}

bool UsesStatProfile(const ItemDefinition& item, std::string_view name)
{
    return IsDefaultProfile(name) ? IsDefaultProfile(item.statProfile) : item.statProfile == name;
}

std::vector<int> FindStatProfileUsers(std::string_view name)
{
    std::vector<int> users;
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        const ItemDefinition* item = g_ItemDatabase.Find(itemType);
        if (item != nullptr && UsesStatProfile(*item, name))
        {
            users.push_back(itemType);
        }
    }
    return users;
}

std::vector<int> FindStatProfileRules(std::string_view name)
{
    const std::vector<Data::Items::StatProfileRule>& rules = g_ItemDatabase.GetStatProfiles().GetRules();
    std::vector<int> found;
    for (size_t index = 0; index < rules.size(); ++index)
    {
        if (CountKey(rules[index].profile) == CountKey(name))
        {
            found.push_back(static_cast<int>(index));
        }
    }
    return found;
}

int ItemStatProfileUseCounts::Items(std::string_view name)
{
    Update();
    return FindCount(m_items, name);
}

int ItemStatProfileUseCounts::Rules(std::string_view name)
{
    Update();
    return FindCount(m_rules, name);
}

const std::vector<int>& ItemStatProfileUseCounts::DescriptionUsers(std::string_view table)
{
    static const std::vector<int> None;
    Update();
    const auto found = m_descriptionUsers.find(table);
    return found != m_descriptionUsers.end() ? found->second : None;
}

void ItemStatProfileUseCounts::Update()
{
    if (m_version == g_ItemDatabase.GetVersion())
    {
        return;
    }
    m_version = g_ItemDatabase.GetVersion();
    m_items.clear();
    m_rules.clear();
    m_descriptionUsers.clear();
    for (const ItemDefinition& item : g_ItemDatabase.GetAllSlots())
    {
        if (item.Exists())
        {
            ++m_items[std::string(CountKey(item.statProfile))];
        }
    }
    for (const Data::Items::StatProfileRule& rule : g_ItemDatabase.GetStatProfiles().GetRules())
    {
        ++m_rules[std::string(CountKey(rule.profile))];
    }
    for (const std::string& table : g_ItemDatabase.GetStatProfiles().GetTableNames())
    {
        m_descriptionUsers[table] = FindTableDescriptionUsers(g_ItemDatabase.GetAllSlots(), table);
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
