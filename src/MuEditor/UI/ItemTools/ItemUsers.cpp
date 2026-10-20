#include "stdafx.h"

#ifdef _EDITOR

#include "ItemUsers.h"

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

std::vector<int> FindStatProfileUsers(std::string_view name)
{
    const bool defaultProfile = IsDefaultProfile(name);
    std::vector<int> users;
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        const ItemDefinition* item = g_ItemDatabase.Find(itemType);
        if (item != nullptr && (defaultProfile ? IsDefaultProfile(item->statProfile) : item->statProfile == name))
        {
            users.push_back(itemType);
        }
    }
    return users;
}
} // namespace MuEditor::Items

#endif // _EDITOR
