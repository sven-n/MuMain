#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/EffectBrowser/EffectPreviewCamera.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewScene.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewTarget.h"
#include "ItemMeshPicking.h"

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
    void Render(int itemType);
    // The model of a slot that is no item's, a part model (design L50), with the same controls.
    void RenderModelSlot(int modelSlot);
    // The most views RenderSideBySide draws.
    static constexpr int MaxViews = 3;
    // Draws what stands above view `view` (0 is the left one), its name or a control, in the view's width; one line.
    using Header = std::function<void(int view, float width)>;
    // Runs `draw`, the drawing of view `view`, inside what that view needs.
    using DrawIn = std::function<void(int view, const std::function<void()>& draw)>;
    // `count` views of the item side by side (at most MaxViews), each below its `header`: one set of controls and one
    // camera, so a drag in any turns all; each is drawn inside `drawIn`.
    void RenderSideBySide(int itemType, int count, const Header& header, const DrawIn& drawIn);

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
                    float height, const std::function<void(const std::function<void()>&)>& drawIn);
    void DrawScene(const MuEditor::Effects::EffectPreviewRequest& request, View& view, float aspect,
                   const std::optional<MeshPicking::Ray>& pickRay);
    void TakeBackPickOnDoubleClick() const;

    MuEditor::Effects::EffectPreviewCamera m_camera;
    std::array<View, MaxViews> m_views;
    int m_framedItemType = -1;
    int m_level = 0;
    bool m_excellent = false;
    bool m_ancient = false;
    bool m_turn = false;
    bool m_asDoppelganger = false;
    MuEditor::Effects::PreviewItemPose m_pose = MuEditor::Effects::PreviewItemPose::Inventory;
    // ImGui::GetTime() of the last click that picked a mesh; long ago before the first one.
    double m_pickTime = std::numeric_limits<double>::lowest();
};
} // namespace MuEditor::Items

#endif // _EDITOR
