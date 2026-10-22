#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/EffectBrowser/EffectPreviewCamera.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewScene.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewTarget.h"

namespace MuEditor::Items
{
// The model tool's view of the selected item's model as it is opened now,
// turning, at an item level, excellent or ancient, upright as the inventory
// turns it or as it lies on the ground: drawn like the effect browser draws an
// item (EffectPreviewScene), into a texture. The mouse turns the camera
// (EffectPreviewInput: the middle button moves the picture, the wheel zooms,
// a double click puts it back).
class CItemModelPreview
{
public:
    void Render(int itemType);

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

    // Between frames: the texture can only be released then.
    void BeforeFrame()
    {
        m_target.BeforeFrame();
    }

private:
    void RenderControls();
    void RenderView(const MuEditor::Effects::EffectPreviewRequest& request);

    MuEditor::Effects::EffectPreviewCamera m_camera;
    MuEditor::Effects::EffectPreviewScene m_scene;
    MuEditor::Effects::EffectPreviewTarget m_target;
    int m_framedItemType = -1;
    int m_level = 0;
    bool m_excellent = false;
    bool m_ancient = false;
    bool m_turn = true;
    MuEditor::Effects::PreviewItemPose m_pose = MuEditor::Effects::PreviewItemPose::Inventory;
};
} // namespace MuEditor::Items

#endif // _EDITOR
