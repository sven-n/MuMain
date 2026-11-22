#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/EffectBrowser/EffectPreviewCamera.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewScene.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewTarget.h"
#include "ItemMeshPicking.h"
#include "imgui.h"

#include <array>
#include <functional>
#include <limits>
#include <optional>

namespace MuEditor::Items
{
// The model tool's view of the selected item's model as it is opened now,
// at an item level, excellent or ancient, upright as the inventory
// turns it or as it lies on the ground, turning when Turn is ticked (off at
// first): drawn like the effect browser draws an item (EffectPreviewScene), into
// a texture. The mouse turns the camera (EffectPreviewInput: the middle button
// moves the picture, the wheel zooms, a double click puts it back). The mesh the
// mesh list or a mesh field shows is drawn alone, the others see-through, and a
// click on a part keeps its mesh (g_ItemMeshFocus). The Style Editor shows three
// views of the item side by side, with one set of controls and one camera.
class CItemModelPreview
{
public:
    // Runs right after view `view` drew its item, with the view's matrices: what the view drew (`request`, turned by
    // `turnDegrees`) and its camera.
    using AfterItem = std::function<void(int view, const MuEditor::Effects::EffectPreviewRequest& request,
                                         float turnDegrees, const MuEditor::Effects::EffectPreviewCamera& camera)>;

    // The item in one view; `afterItem` (if set) runs in it, as view 0.
    void Render(int itemType, const AfterItem& afterItem = {});
    // The model of a slot that is no item's, a part model (design L50), with the same controls.
    void RenderModelSlot(int modelSlot);
    // The most views RenderSideBySide draws.
    static constexpr int MaxViews = 3;
    // Draws what stands above view `view` (0 is the left one), its name or a control, in the view's width; one line.
    using Header = std::function<void(int view, float width)>;
    // Runs `draw`, the drawing of view `view`, inside what that view needs.
    using DrawIn = std::function<void(int view, const std::function<void()>& draw)>;
    // Where a view's picture stands in the window, and the point a left click that did not turn it clicked.
    struct ViewOnScreen
    {
        ImVec2 min;
        ImVec2 max;
        std::optional<ImVec2> click;
        bool hovered = false;
    };
    // Runs right after view `view`'s picture is put into the window, to draw over it: what the view drew (`request`,
    // turned by `turnDegrees`), its camera and where it stands.
    using Overlay =
        std::function<void(int view, const MuEditor::Effects::EffectPreviewRequest& request, float turnDegrees,
                           const MuEditor::Effects::EffectPreviewCamera& camera, const ViewOnScreen& screen)>;
    // `count` views of the item side by side (at most MaxViews), each below its `header`: one set of controls and one
    // camera, so a drag in any turns all; each is drawn inside `drawIn`, `afterItem` (if set) runs in each and
    // `overlay` (if set) over each.
    void RenderSideBySide(int itemType, int count, const Header& header, const DrawIn& drawIn,
                          const AfterItem& afterItem = {}, const Overlay& overlay = {});
    // Whether a click in a view keeps the mesh it hits (on at first); off while a click means something else.
    void SetMeshPicking(bool picking)
    {
        m_meshPicking = picking;
    }

    // The item level and looks the controls chose, for the inventory grid.
    int GetLevel() const
    {
        return m_level;
    }
    bool IsExcellent() const
    {
        return m_excellent;
    }
    bool IsAncient() const
    {
        return m_ancient;
    }
    // Draws the item as a doppelganger's (the Style Editor's "Draw as: doppelganger").
    void SetDrawAsDoppelganger(bool doppelganger)
    {
        m_asDoppelganger = doppelganger;
    }
    // Whether the preview draws the item's own model at the chosen level, the one whose meshes the mesh list lists.
    [[nodiscard]] bool ShowsListedModel(int itemType) const;

    // Between frames: the textures can only be released then.
    void BeforeFrame()
    {
        for (View& view : m_views)
        {
            view.target.BeforeFrame();
        }
    }

private:
    // What each view draws with: a scene of its own (its own turn) and a texture of its own.
    struct View
    {
        MuEditor::Effects::EffectPreviewScene scene;
        MuEditor::Effects::EffectPreviewTarget target;
    };

    MuEditor::Effects::EffectPreviewRequest MakeRequest(int itemType) const;
    void RenderControls();
    // The view's height for a width the window gives it.
    [[nodiscard]] float ViewHeight(float widthInWindow) const;
    void RenderView(const MuEditor::Effects::EffectPreviewRequest& request, View& view, const char* id, float width,
                    float height, const std::function<void(const std::function<void()>&)>& drawIn,
                    const std::function<void()>& afterItem = {},
                    const std::function<void(const ViewOnScreen&)>& overlay = {});
    void DrawScene(const MuEditor::Effects::EffectPreviewRequest& request, View& view, float aspect,
                   const std::optional<MeshPicking::Ray>& pickRay, const std::function<void()>& afterItem);
    void TakeBackPickOnDoubleClick() const;

    MuEditor::Effects::EffectPreviewCamera m_camera;
    std::array<View, MaxViews> m_views;
    int m_framedItemType = -1;
    int m_level = 0;
    bool m_excellent = false;
    bool m_ancient = false;
    bool m_turn = false;
    bool m_asDoppelganger = false;
    bool m_meshPicking = true;
    MuEditor::Effects::PreviewItemPose m_pose = MuEditor::Effects::PreviewItemPose::Inventory;
    // ImGui::GetTime() of the last click that picked a mesh; long ago before the first one.
    double m_pickTime = std::numeric_limits<double>::lowest();
};
} // namespace MuEditor::Items

#endif // _EDITOR
