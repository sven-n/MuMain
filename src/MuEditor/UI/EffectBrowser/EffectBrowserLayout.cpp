#include "stdafx.h"

#ifdef _EDITOR

#include "EffectBrowserLayout.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "imgui.h"

namespace MuEditor::Effects::Layout
{
void SameLineIfFits(float width)
{
    ImGui::SameLine();
    if (ImGui::GetContentRegionAvail().x < width)
        ImGui::NewLine();
}

// A label in front of a combo.
float LabeledComboWidth(const char* label, float comboWidth)
{
    return ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemSpacing.x + comboWidth * g_MuEditorCore.GetUIScale();
}

float CheckboxWidth(const char* label)
{
    return ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize(label).x;
}

float ButtonWidth(const char* label)
{
    return ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2.0f;
}
} // namespace MuEditor::Effects::Layout

#endif // _EDITOR
