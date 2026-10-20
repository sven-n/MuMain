#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/EffectBrowser/EffectPreviewTarget.h"

namespace MuEditor::Items
{
// The model tool's picture of the selected item in an inventory grid: its
// width x height cells with a cell around them, and the item drawn as the
// game's inventory draws it (RenderItem3D under the original's item camera,
// UI::Items::DrawUnderItemCamera), with the inventory display the game shows
// now, also while a slider is dragged. It turns while the mouse is on it, as
// in the game.
class CItemInventoryGridPreview
{
public:
    void Render(int itemType, int level, bool excellent, bool ancient);

    // Between frames: the texture can only be released then.
    void BeforeFrame()
    {
        m_target.BeforeFrame();
    }

private:
    // Its own depth buffer: the model preview draws in the same frames.
    MuEditor::Effects::EffectPreviewTarget m_target{MuEditor::Effects::PreviewDepth::Own};
};
} // namespace MuEditor::Items

#endif // _EDITOR
