#include "stdafx.h"

#ifdef _EDITOR

#include "ItemModelPreview.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewInput.h"
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
    request.itemPose = m_pose;
    request.itemGroundPlane = true;
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
    // The pose of the display values the sections below edit; their headers
    // have the same labels.
    using MuEditor::Effects::PreviewItemPose;
    ImGui::PushID("Pose");
    if (ImGui::RadioButton(I18N::Editor::InTheInventory, m_pose == PreviewItemPose::Inventory))
    {
        m_pose = PreviewItemPose::Inventory;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(I18N::Editor::OnTheGround, m_pose == PreviewItemPose::Ground))
    {
        m_pose = PreviewItemPose::Ground;
    }
    ImGui::PopID();
}

void CItemModelPreview::RenderView(const MuEditor::Effects::EffectPreviewRequest& request)
{
    const float width = ImGui::GetContentRegionAvail().x;
    if (width < 1.0f)
    {
        return;
    }
    const float scale = g_MuEditorCore.GetUIScale();
    // The width without the vertical scrollbar, so the height does not make
    // the scrollbar show and hide by turns (as the effect browser's view).
    const ImGuiStyle& style = ImGui::GetStyle();
    const float widthWithoutScrollbar = ImGui::GetWindowWidth() - 2.0f * style.WindowPadding.x - style.ScrollbarSize;
    const float height = std::clamp(widthWithoutScrollbar * ViewAspect, MinViewHeight * scale, MaxViewHeight * scale);
    MuEditor::Effects::PreviewInput::RenderView("##ModelView", width, height, m_camera);
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
} // namespace MuEditor::Items

#endif // _EDITOR
