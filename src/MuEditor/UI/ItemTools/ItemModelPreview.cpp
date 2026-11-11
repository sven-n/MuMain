#include "stdafx.h"

#ifdef _EDITOR

#include "ItemModelPreview.h"

#include "ItemMeshFocus.h"
#include "ItemMeshPreviewDrawer.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewInput.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewItems.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewSubject.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "I18N/All.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Renderer/MuRenderer.h"
#include "UI/Common/ScopedOffscreenCapture.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>

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

// After the view: a left click that did not turn it gives the ray through the clicked point. The second click of a
// double click puts the view back (EffectPreviewInput) and gives none.
std::optional<MeshPicking::Ray> ClickedRay(const MuEditor::Effects::EffectPreviewCamera& camera, const ImVec2& min,
                                           const ImVec2& max)
{
    if (!ImGui::IsItemDeactivated() || !ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        return std::nullopt;
    }
    const ImGuiIO& io = ImGui::GetIO();
    if (io.MouseClickedLastCount[ImGuiMouseButton_Left] > 1)
    {
        return std::nullopt;
    }
    const float threshold = io.MouseDragThreshold;
    if (io.MouseDragMaxDistanceSqr[ImGuiMouseButton_Left] > threshold * threshold)
    {
        return std::nullopt;
    }
    const float width = max.x - min.x;
    const float height = max.y - min.y;
    if (width <= 0.0f || height <= 0.0f)
    {
        return std::nullopt;
    }
    const float viewX = (io.MousePos.x - min.x) / width * 2.0f - 1.0f;
    const float viewY = 1.0f - (io.MousePos.y - min.y) / height * 2.0f;
    if (std::abs(viewX) > 1.0f || std::abs(viewY) > 1.0f)
    {
        return std::nullopt;
    }
    return MeshPicking::RayThroughView(camera, width / height, viewX, viewY);
}

// The item's own model, which the mesh list lists, when the preview draws it at `level`; nothing for a level drawn
// with another model (the Box of Luck's levels, (14,12) at +0 and +2), which has none of the listed meshes.
const BMD* ListedModelInView(int itemType, int level)
{
    // A part model's view (RenderModelSlot) has no item.
    if (!Data::Items::IsValidItemType(itemType))
    {
        return nullptr;
    }
    const int listed = Data::Items::ToModelSlot(itemType);
    if (MuEditor::Effects::GetPreviewItemModels(itemType, level).model != listed)
    {
        return nullptr;
    }
    return &Models[listed];
}
} // namespace

bool CItemModelPreview::ShowsListedModel(int itemType) const
{
    return ListedModelInView(itemType, m_level) != nullptr;
}

void CItemModelPreview::Render(int itemType)
{
    RenderControls();
    const MuEditor::Effects::EffectPreviewRequest request = MakeRequest(itemType);
    const float width = ImGui::GetContentRegionAvail().x;
    // The width without the vertical scrollbar, so the height does not make
    // the scrollbar show and hide by turns (as the effect browser's view).
    const ImGuiStyle& style = ImGui::GetStyle();
    const float widthInWindow = ImGui::GetWindowWidth() - 2.0f * style.WindowPadding.x - style.ScrollbarSize;
    RenderView(request, m_views[0], "##ModelView", width, ViewHeight(widthInWindow), nullptr);
}

void CItemModelPreview::RenderModelSlot(int modelSlot)
{
    RenderControls();
    // An item type no item has, one per slot, so the camera frames each slot on its own.
    MuEditor::Effects::EffectPreviewRequest request = MakeRequest(-2 - modelSlot);
    request.modelSlot = modelSlot;
    const float width = ImGui::GetContentRegionAvail().x;
    const ImGuiStyle& style = ImGui::GetStyle();
    const float widthInWindow = ImGui::GetWindowWidth() - 2.0f * style.WindowPadding.x - style.ScrollbarSize;
    RenderView(request, m_views[0], "##ModelView", width, ViewHeight(widthInWindow), nullptr);
}

void CItemModelPreview::RenderSideBySide(int itemType, int count, const Header& header, const DrawIn& drawIn)
{
    RenderControls();
    count = std::clamp(count, 1, MaxViews);
    const MuEditor::Effects::EffectPreviewRequest request = MakeRequest(itemType);
    const ImGuiStyle& style = ImGui::GetStyle();
    const float gaps = style.ItemSpacing.x * static_cast<float>(count - 1);
    const float width = (ImGui::GetContentRegionAvail().x - gaps) / static_cast<float>(count);
    const float widthInWindow = (ImGui::GetWindowWidth() - 2.0f * style.WindowPadding.x - style.ScrollbarSize - gaps) /
                                static_cast<float>(count);
    const float height = ViewHeight(widthInWindow);
    for (int view = 0; view < count; ++view)
    {
        if (view > 0)
        {
            ImGui::SameLine();
        }
        ImGui::BeginGroup();
        ImGui::PushID(view);
        // Each header gets the height of a control, so the views stay in line under a label beside a dropdown.
        const float top = ImGui::GetCursorPosY();
        header(view, width);
        ImGui::SetCursorPosY(std::max(ImGui::GetCursorPosY(), top + ImGui::GetFrameHeightWithSpacing()));
        RenderView(request, m_views[static_cast<std::size_t>(view)], "##ModelView", width, height,
                   [&drawIn, view](const std::function<void()>& draw) { drawIn(view, draw); });
        ImGui::PopID();
        ImGui::EndGroup();
    }
}

MuEditor::Effects::EffectPreviewRequest CItemModelPreview::MakeRequest(int itemType) const
{
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
    request.itemAsDoppelganger = m_asDoppelganger;
    return request;
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

float CItemModelPreview::ViewHeight(float widthInWindow) const
{
    const float scale = g_MuEditorCore.GetUIScale();
    return std::clamp(widthInWindow * ViewAspect, MinViewHeight * scale, MaxViewHeight * scale);
}

void CItemModelPreview::RenderView(const MuEditor::Effects::EffectPreviewRequest& request, View& view, const char* id,
                                   float width, float height,
                                   const std::function<void(const std::function<void()>&)>& drawIn)
{
    if (width < 1.0f)
    {
        return;
    }
    MuEditor::Effects::PreviewInput::RenderView(id, width, height, m_camera);
    TakeBackPickOnDoubleClick();
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    const std::optional<MeshPicking::Ray> pickRay = ClickedRay(m_camera, min, max);
    if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
    {
        // Over the effect browser's controls text, which RenderView set.
        ImGui::SetTooltip("%s %s", I18N::Editor::PreviewControls, I18N::Editor::PreviewPickNote);
    }
    const MuEditor::Effects::PreviewFraming framing = view.scene.Prepare(request);
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
        texture = view.target.Begin(size.width, size.height, !ImGui::IsMouseDown(ImGuiMouseButton_Left));
        if (texture == 0)
        {
            return;
        }
        const MuEditor::ScopedOffscreenCapture endCapture;
        const auto draw = [&] { DrawScene(request, view, width / height, pickRay); };
        if (drawIn)
        {
            drawIn(draw);
        }
        else
        {
            draw();
        }
    }
    else
    {
        // The game skipped this frame; the editor's windows may still be drawn.
        texture = view.target.KeepLastPicture();
    }
    if (void* pointer = texture != 0 ? mu::GetRenderer().GetTexturePointer(texture) : nullptr)
    {
        ImGui::GetWindowDrawList()->AddImage((ImTextureID)(intptr_t)pointer, min, max);
    }
}

void CItemModelPreview::DrawScene(const MuEditor::Effects::EffectPreviewRequest& request, View& view, float aspect,
                                  const std::optional<MeshPicking::Ray>& pickRay)
{
    const BMD* model = ListedModelInView(request.itemType, request.itemLevel);
    std::optional<int> hitMesh;
    {
        // Without the listed model the drawer leaves every draw alone.
        CItemMeshPreviewDrawer drawer(model, g_ItemMeshFocus.Shown(), pickRay);
        view.scene.Draw(request, m_camera, aspect);
        hitMesh = drawer.HitMesh();
    }
    // A click on another model finds none of the listed meshes, so it keeps the kept one.
    if (pickRay && model != nullptr)
    {
        g_ItemMeshFocus.Pick(hitMesh);
        m_pickTime = ImGui::GetTime();
    }
}

// After the view, as EffectPreviewInput puts the view back on a double click: the mesh its first click picked is
// taken back, so the double click leaves the kept mesh as it was.
void CItemModelPreview::TakeBackPickOnDoubleClick() const
{
    if (!ImGui::IsItemHovered() || !ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        return;
    }
    if (ImGui::GetTime() - m_pickTime <= ImGui::GetIO().MouseDoubleClickTime)
    {
        g_ItemMeshFocus.TakeBackPick();
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
