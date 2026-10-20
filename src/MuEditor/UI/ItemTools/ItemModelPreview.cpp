#include "stdafx.h"

#ifdef _EDITOR

#include "ItemModelPreview.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewSubject.h"
#include "I18N/All.h"
#include "Render/Renderer/MuRenderer.h"
#include "UI/Common/ScopedOffscreenCapture.h"
#include "imgui.h"

#include <algorithm>
#include <cstdint>

namespace MuEditor::Items
{
namespace
{
// The view's height to its width, and its height before the editor UI scale.
constexpr float ViewAspect = 0.75f;
constexpr float MinViewHeight = 160.0f;
constexpr float MaxViewHeight = 360.0f;
constexpr int MaxItemLevel = 15;
constexpr float LevelWidth = 120.0f;
} // namespace

void CItemModelPreview::Render(int itemType)
{
    RenderControls();
    MuEditor::Effects::EffectPreviewRequest request;
    // Not an effect: the scene draws the item alone.
    request.kind = Data::Effects::EffectKind::Particle;
    request.type = -1;
    request.subject.draw = MuEditor::Effects::PreviewDraw::None;
    request.showOn = MuEditor::Effects::PreviewShowOn::Item;
    request.itemType = itemType;
    request.itemLevel = m_level;
    request.itemExcellent = m_excellent;
    request.itemAncient = m_ancient;
    request.turn = m_turn;
    RenderView(request);
}

void CItemModelPreview::RenderControls()
{
    ImGui::SetNextItemWidth(LevelWidth * g_MuEditorCore.GetUIScale());
    ImGui::SliderInt(I18N::Metadata::Level, &m_level, 0, MaxItemLevel, "+%d");
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::Excellent, &m_excellent);
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::Ancient, &m_ancient);
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::Turn, &m_turn);
}

void CItemModelPreview::RenderView(const MuEditor::Effects::EffectPreviewRequest& request)
{
    const float width = ImGui::GetContentRegionAvail().x;
    if (width < 1.0f)
    {
        return;
    }
    const float scale = g_MuEditorCore.GetUIScale();
    const float height = std::clamp(width * ViewAspect, MinViewHeight * scale, MaxViewHeight * scale);
    ImGui::InvisibleButton("##ModelView", ImVec2(width, height), ImGuiButtonFlags_MouseButtonLeft);
    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
    HandleViewInput();
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::PreviewControls);
    }
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    const MuEditor::Effects::PreviewFraming framing = m_scene.Prepare(request);
    if (m_framedItemType != request.itemType)
    {
        m_framedItemType = request.itemType;
        m_camera.Frame(framing.center, framing.radius);
    }
    if (!ImGui::IsItemVisible())
    {
        return;
    }
    std::uint32_t texture = 0;
    if (mu::GetRenderer().IsFrameActive())
    {
        // The density of the OS window the view is in; ImGui leaves it 0
        // while the editor's windows stay in the game window.
        const float viewportScale = ImGui::GetWindowViewport()->FramebufferScale.x;
        const float framebufferScale = viewportScale > 0.0f ? viewportScale : ImGui::GetIO().DisplayFramebufferScale.x;
        const MuEditor::Effects::PreviewTextureSize size =
            MuEditor::Effects::ChoosePreviewTextureSize(width, height, framebufferScale);
        texture = m_target.Begin(size.width, size.height, !ImGui::IsMouseDown(ImGuiMouseButton_Left));
        if (texture == 0)
        {
            return;
        }
        const MuEditor::ScopedOffscreenCapture endCapture;
        m_scene.Draw(request, m_camera, width / height);
    }
    else
    {
        // The game skipped this frame; the editor's windows may still be drawn.
        texture = m_target.KeepLastPicture();
    }
    if (void* pointer = texture != 0 ? mu::GetRenderer().GetTexturePointer(texture) : nullptr)
    {
        ImGui::GetWindowDrawList()->AddImage((ImTextureID)(intptr_t)pointer, min, max);
    }
}

void CItemModelPreview::HandleViewInput()
{
    const ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
    {
        m_camera.Turn(io.MouseDelta.x, io.MouseDelta.y);
    }
    if (!ImGui::IsItemHovered())
    {
        return;
    }
    if (io.MouseWheel != 0.0f)
    {
        m_camera.Zoom(io.MouseWheel);
    }
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        m_camera.Reset();
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
