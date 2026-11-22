#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterLookPicker.h"

#include "../MuEditor/UI/StyleEditor/MuStyleEditorUI.h"
#include "Data/GameData/LookData/LookStore.h"
#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Monsters::LookPicker
{
namespace
{
constexpr ImVec4 WarningColor{1.0f, 0.75f, 0.3f, 1.0f};
} // namespace

bool Render(std::string& name, Data::Looks::LookKind kind, const std::string& noLook, const char* noLookLabel,
            float width)
{
    bool changed = false;
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo("##Look", name == noLook ? noLookLabel : name.c_str()))
    {
        if (ImGui::Selectable(noLookLabel, name == noLook) && name != noLook)
        {
            name = noLook;
            changed = true;
        }
        for (const Data::Looks::LookDefinition& look : Data::Looks::g_LookStore.GetLooks(kind))
        {
            if (ImGui::Selectable(look.name.c_str(), look.name == name) && look.name != name)
            {
                name = look.name;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    const bool known = Data::Looks::g_LookStore.Find(kind, name) != nullptr;
    ImGui::SameLine();
    ImGui::BeginDisabled(!known);
    if (ImGui::SmallButton(I18N::Editor::OpenInStyleEditor))
    {
        if (kind == Data::Looks::LookKind::Effect)
        {
            g_MuStyleEditorUI.ShowEffect(name);
        }
        else
        {
            g_MuStyleEditorUI.ShowRenderStyle(name);
        }
    }
    ImGui::EndDisabled();
    if (!known && name != noLook)
    {
        ImGui::SameLine();
        ImGui::TextColored(WarningColor, "%s", I18N::Format(I18N::Editor::UnknownName, {name}).c_str());
    }
    return changed;
}
} // namespace MuEditor::Monsters::LookPicker

#endif // _EDITOR
