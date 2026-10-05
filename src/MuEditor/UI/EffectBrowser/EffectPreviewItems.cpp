#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPreviewItems.h"

#include "EffectBrowserAssets.h"
#include "EffectBrowserModel.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Render/Items/ItemDisplay.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <utility>

namespace MuEditor::Effects
{
void EffectPreviewItems::Build(int itemCount, const NameOf& nameOf, const IsDrawable& drawable)
{
    m_items.clear();
    for (int itemType = 0; itemType < itemCount; ++itemType)
    {
        std::string name = nameOf(itemType);
        if (name.empty() || !drawable(itemType))
            continue;
        std::string searchText = ToSearchText(name) + ' ' + std::to_string(itemType);
        m_items.push_back({itemType, std::move(name), std::move(searchText)});
    }
}

const PreviewItem* EffectPreviewItems::Find(int itemType) const
{
    const auto found = std::find_if(m_items.begin(), m_items.end(),
                                    [itemType](const PreviewItem& item) { return item.type == itemType; });
    return found != m_items.end() ? &*found : nullptr;
}

std::vector<int> EffectPreviewItems::Filter(std::string_view search) const
{
    const std::string lower = ToSearchText(search);
    std::vector<int> listed;
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i].searchText.find(lower) != std::string::npos)
            listed.push_back(static_cast<int>(i));
    }
    return listed;
}

bool PreviewItemModels::CanDraw() const
{
    return IsModelLoaded(model) && skeleton >= 0 && Models[skeleton].NumActions > 0;
}

PreviewItemModels GetPreviewItemModels(int itemType, int level)
{
    const int model = Render::Items::Display::GetDrawnModel(Data::Items::ToModelSlot(itemType), level);
    const int skeleton = Render::Items::Display::IsDrawnOnCharacterSkeleton(model) ? MODEL_PLAYER : model;
    return {model, skeleton};
}
} // namespace MuEditor::Effects

#endif // _EDITOR
