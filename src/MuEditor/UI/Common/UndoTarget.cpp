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

// What the Undo or the Redo button asks of its target.
struct ButtonActions
{
    bool (UndoTarget::*can)() const;
    std::string (UndoTarget::*describe)() const;
    void (UndoTarget::*act)();
};

const ButtonActions UndoButton{&UndoTarget::CanUndo, &UndoTarget::GetUndoLabel, &UndoTarget::Undo};
const ButtonActions RedoButton{&UndoTarget::CanRedo, &UndoTarget::GetRedoLabel, &UndoTarget::Redo};

// The step is described only while its tooltip shows: the bar is drawn every
// frame, the tooltip rarely.
void RenderButton(const char* label, const char* keys, const ButtonActions& actions, UndoTarget& target)
{
    ImGui::BeginDisabled(!(target.*actions.can)());
    if (ImGui::Button(label))
    {
        (target.*actions.act)();
    }
    // Asked again: after a click there may be no step left.
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && (target.*actions.can)())
    {
        ImGui::SetTooltip("%s (%s)", (target.*actions.describe)().c_str(), keys);
    }
    ImGui::EndDisabled();
}
} // namespace

void Activate(UndoTarget& target)
{
    g_activeTarget = &target;
}

UndoTarget& GetCurrent(UndoTarget& fallback)
{
    const bool activeIsOpen = g_activeTarget != nullptr && g_activeTarget->IsOpen();
    return activeIsOpen ? *g_activeTarget : fallback;
}

void RenderBarButtons(UndoTarget& fallback)
{
    UndoTarget& target = GetCurrent(fallback);
    // A click leaves the focus in the tool, where Ctrl+Z and Ctrl+Y work.
    ImGui::PushItemFlag(ImGuiItemFlags_NoFocus, true);
    RenderButton(I18N::Editor::Undo, "Ctrl+Z", UndoButton, target);
    ImGui::SameLine();
    RenderButton(I18N::Editor::Redo, "Ctrl+Y", RedoButton, target);
    ImGui::PopItemFlag();
}
} // namespace MuEditor::UndoTargets

#endif // _EDITOR
