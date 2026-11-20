#include "stdafx.h"

#ifdef _EDITOR

#include "ItemToolWidgets.h"

#include "ItemSelection.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "I18N/All.h"

#include <cfloat>

namespace MuEditor::Items::ToolWidgets
{
namespace
{
constexpr float ItemListLines = 8.0f;
} // namespace

void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}

bool SmallButtonUnlessUnapplied(const char* label, bool unapplied)
{
    ImGui::BeginDisabled(unapplied);
    const bool clicked = ImGui::SmallButton(label);
    ImGui::EndDisabled();
    if (unapplied && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::ApplyOrRevertTheChangesFirst);
    }
    return clicked;
}

void RenderItemList(const char* id, const std::vector<int>& items)
{
    if (!ImGui::BeginListBox(id, ImVec2(-FLT_MIN, ItemListLines * ImGui::GetTextLineHeightWithSpacing())))
    {
        return;
    }
    for (const int item : items)
    {
        ImGui::PushID(item);
        if (ImGui::Selectable(g_ItemDatabase.GetLogName(item).c_str(), item == g_ItemSelection.GetItemType()))
        {
            g_ItemSelection.Select(item);
        }
        ImGui::PopID();
    }
    ImGui::EndListBox();
}
} // namespace MuEditor::Items::ToolWidgets

#endif // _EDITOR
