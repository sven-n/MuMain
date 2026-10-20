#include "stdafx.h"

#ifdef _EDITOR

#include "ItemListFields.h"

#include "ItemTextInput.h"

#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Items::ListFields
{
namespace
{
constexpr float FolderWidth = 240.0f;
} // namespace

bool EditFolders(const char* id, std::vector<std::string>& folders, const std::function<void()>& afterField)
{
    ImGui::PushID(id);
    bool changed = false;
    for (size_t index = 0; index < folders.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        ImGui::SetNextItemWidth(FolderWidth);
        InputString("##Folder", folders[index]);
        if (afterField)
        {
            afterField();
        }
        else
        {
            changed = ImGui::IsItemDeactivatedAfterEdit() || changed;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("x"))
        {
            folders.erase(folders.begin() + static_cast<std::ptrdiff_t>(index));
            changed = true;
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }
    if (ImGui::SmallButton(I18N::Editor::AddAFolder))
    {
        folders.emplace_back("Item");
        changed = true;
    }
    ImGui::PopID();
    return changed;
}
} // namespace MuEditor::Items::ListFields

#endif // _EDITOR
