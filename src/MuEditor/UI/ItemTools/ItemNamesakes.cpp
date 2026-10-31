#include "stdafx.h"

#ifdef _EDITOR

#include "ItemNamesakes.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "imgui.h"

#include <map>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDefinition;
using Data::Items::ItemTag;

// The tags of the item that not every one of `namesakes` has; a tag that only leaves a tooltip line out says nothing
// about what the item is.
std::string DistinguishingTags(const ItemDefinition& item, const std::vector<int>& namesakes)
{
    std::string text;
    for (const auto& tag : Data::Items::GetEnumNames(ItemTag{}))
    {
        if (!item.tags.Has(tag.value) || Data::Items::IsTooltipOnlyTag(tag.value))
        {
            continue;
        }
        bool everyOneHasIt = true;
        for (const int other : namesakes)
        {
            everyOneHasIt = everyOneHasIt && g_ItemDatabase.GetAllSlots()[other].tags.Has(tag.value);
        }
        if (!everyOneHasIt)
        {
            text += (text.empty() ? "" : ", ") + std::string(tag.name);
        }
    }
    return text;
}
} // namespace

ItemNamesakes& ItemNamesakes::GetInstance()
{
    static ItemNamesakes instance;
    return instance;
}

const std::string& ItemNamesakes::GetQualifier(int itemType)
{
    static const std::string None;
    Update();
    if (itemType < 0 || static_cast<size_t>(itemType) >= m_qualifiers.size())
    {
        return None;
    }
    return m_qualifiers[static_cast<size_t>(itemType)];
}

void ItemNamesakes::Update()
{
    if (m_version == g_ItemDatabase.GetVersion() && m_nameVersion == g_ItemDatabase.GetNameVersion())
    {
        return;
    }
    m_version = g_ItemDatabase.GetVersion();
    m_nameVersion = g_ItemDatabase.GetNameVersion();
    const auto slots = g_ItemDatabase.GetAllSlots();
    m_qualifiers.assign(slots.size(), std::string());

    // In item type order, so the counting goes by number.
    std::map<std::wstring, std::vector<int>> byName;
    for (size_t itemType = 0; itemType < slots.size(); ++itemType)
    {
        if (slots[itemType].Exists() && !slots[itemType].name.empty())
        {
            byName[slots[itemType].name].push_back(static_cast<int>(itemType));
        }
    }
    for (const auto& [name, namesakes] : byName)
    {
        if (namesakes.size() < 2)
        {
            continue;
        }
        std::map<std::string, std::vector<int>> byTags;
        for (const int itemType : namesakes)
        {
            byTags[DistinguishingTags(slots[static_cast<size_t>(itemType)], namesakes)].push_back(itemType);
        }
        for (const auto& [tags, items] : byTags)
        {
            for (size_t index = 0; index < items.size(); ++index)
            {
                std::string qualifier = tags;
                if (items.size() > 1)
                {
                    qualifier += (qualifier.empty() ? "#" : " #") + std::to_string(index + 1);
                }
                m_qualifiers[static_cast<size_t>(items[index])] = std::move(qualifier);
            }
        }
    }
}

void RenderNamesakeQualifier(int itemType)
{
    const std::string& qualifier = g_ItemNamesakes.GetQualifier(itemType);
    if (!qualifier.empty())
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", qualifier.c_str());
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
