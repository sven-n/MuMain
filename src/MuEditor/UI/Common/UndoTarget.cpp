#include "stdafx.h"

#ifdef _EDITOR

#include "UndoTarget.h"

#include "I18N/All.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace MuEditor::UndoTargets
{
namespace
{
UndoTarget* g_activeTarget = nullptr;

void RenderButton(const char* label, const char* keys, bool enabled, const std::string& step, void (UndoTarget::*act)(),
                  UndoTarget& target)
{
    ImGui::BeginDisabled(!enabled);
    if (ImGui::Button(label))
    {
        (target.*act)();
    }
    if (enabled && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s (%s)", step.c_str(), keys);
    }
    ImGui::EndDisabled();
}
} // namespace

void Activate(UndoTarget& target)
{
    g_activeTarget = &target;
}

void RenderBarButtons(UndoTarget& fallback)
{
    const bool activeIsOpen = g_activeTarget != nullptr && g_activeTarget->IsOpen();
    UndoTarget& target = activeIsOpen ? *g_activeTarget : fallback;
    // A click leaves the focus in the tool, where Ctrl+Z and Ctrl+Y work.
    ImGui::PushItemFlag(ImGuiItemFlags_NoFocus, true);
    const bool canUndo = target.CanUndo();
    RenderButton(I18N::Editor::Undo, "Ctrl+Z", canUndo, canUndo ? target.GetUndoLabel() : std::string(),
                 &UndoTarget::Undo, target);
    ImGui::SameLine();
    const bool canRedo = target.CanRedo();
    RenderButton(I18N::Editor::Redo, "Ctrl+Y", canRedo, canRedo ? target.GetRedoLabel() : std::string(),
                 &UndoTarget::Redo, target);
    ImGui::PopItemFlag();
}
} // namespace MuEditor::UndoTargets

#endif // _EDITOR
