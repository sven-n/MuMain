#include "stdafx.h"

#ifdef _EDITOR

#include "ItemGroupFilter.h"

#include "ItemGroupNames.h"

#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <string>

namespace MuEditor::Items
{
void RenderGroupFilter(std::optional<int>& group, float width)
{
    ImGui::SetNextItemWidth(width);
    const std::string shown = group ? GetItemGroupLabel(*group) : I18N::Editor::AllGroups;
    if (!ImGui::BeginCombo("##Group", shown.c_str()))
    {
        return;
    }
    if (ImGui::Selectable(I18N::Editor::AllGroups, !group))
    {
        group.reset();
    }
    for (int index = 0; index < MAX_ITEM_TYPE; ++index)
    {
        if (ImGui::Selectable(GetItemGroupLabel(index).c_str(), group == index))
        {
            group = index;
        }
    }
    ImGui::EndCombo();
}
} // namespace MuEditor::Items

#endif // _EDITOR
