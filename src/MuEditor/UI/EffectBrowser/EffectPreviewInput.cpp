#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPreviewInput.h"

#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Effects::PreviewInput
{
namespace
{
void HandleInput(float height, EffectPreviewCamera& camera)
{
    const ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsItemActive())
    {
        // One at a time while both buttons are held.
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
        {
            camera.Pan(io.MouseDelta.x, io.MouseDelta.y, height);
        }
        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
        {
            camera.Turn(io.MouseDelta.x, io.MouseDelta.y);
        }
    }
    if (!ImGui::IsItemHovered())
    {
        return;
    }
    if (io.MouseWheel != 0.0f)
    {
        camera.Zoom(io.MouseWheel);
    }
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        camera.Reset();
    }
}
} // namespace

void RenderView(const char* id, float width, float height, EffectPreviewCamera& camera)
{
    ImGui::InvisibleButton(id, ImVec2(width, height),
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);
    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
    HandleInput(height, camera);
    // Not while dragging: it would cover the picture.
    if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
    {
        ImGui::SetTooltip("%s", I18N::Editor::PreviewControls);
    }
}
} // namespace MuEditor::Effects::PreviewInput

#endif // _EDITOR
