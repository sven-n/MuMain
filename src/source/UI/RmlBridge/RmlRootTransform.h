#pragma once

#include "UI/RmlBridge/RmlModelBinder.h"

#include "UI/Scaling/UITransform.h"

extern unsigned int WindowWidth;
extern unsigned int WindowHeight;

// Shared "root transform" sync for every RmlUi window whose position tracks a native, legacy-
// reference-space POINT (a movable/HUD window overlaying live 3D content) rather than a static/
// centered dialog scaled purely by the RmlUi context's own density-independent-pixel ratio -- see
// base.rcss's "px companions" comment (my_inventory.rcss's header-rail family) for why these two
// window families need different CSS units too.
//
// `Model` must expose public `float rootX, rootY, rootScale` members bound to
// "root_x"/"root_y"/"root_scale" respectively (same shape as MyInventoryRmlModel/
// MyInventoryBgRmlModel in MyInventory.h) -- this only writes those three fields and marks them
// dirty, it doesn't touch anything else in the model.
namespace UI::RmlBridge
{
// Native text size in screen pixels at the one typography scale, whatever the window's layout.
inline float NativeTextPx(UI::Scaling::FontRole role)
{
    return UI::Scaling::NativeTextPixelSize(role, static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
}
// The same for a text drawn into a box, shrunk to fit it (UI::Scaling::NativeTextPixelSizeInBox()).
inline float NativeTextPxInBox(UI::Scaling::FontRole role, float measuredWidth, float boxWidth)
{
    return UI::Scaling::NativeTextPixelSizeInBox(role, static_cast<int>(WindowWidth), static_cast<int>(WindowHeight),
                                                 measuredWidth, boxWidth);
}

    template <typename Model>
    void SyncRootTransform(RmlModelBinder<Model>& binder, const POINT& pos)
    {
        const auto transform = UI::Scaling::GetActiveTransform();
        const float rootX = static_cast<float>(pos.x) * transform.scaleX + transform.offsetX;
        const float rootY = static_cast<float>(pos.y) * transform.scaleY + transform.offsetY;
        const float rootScale = transform.scaleX;

        // Dirty only on change, like SyncNativeTextSize() below. This is the most-called sync in the
        // UI -- every docked and inventory window, every frame -- and each dirtied root_* re-runs
        // every view bound to it, which on a window that binds its layout is dozens of writes for a
        // panel that has not moved. The three change together, so one comparison covers them.
        Model& model = binder.GetModel();
        if (model.rootX == rootX && model.rootY == rootY && model.rootScale == rootScale)
            return;

        model.rootX = rootX;
        model.rootY = rootY;
        model.rootScale = rootScale;
        binder.MarkDirty("root_x");
        binder.MarkDirty("root_y");
        binder.MarkDirty("root_scale");
    }

    // Legacy-theme text that must match the native text renderer's size (and be rasterised at it)
    // takes this physical size and counter-scales itself out of the root transform -- see
    // character_info.rml's header comment. `Model` must expose `float textPx` bound to "text_px".
    template <typename Model> void SyncNativeTextSize(RmlModelBinder<Model>& binder)
    {
        const float textPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal,
                                                              static_cast<int>(WindowWidth),
                                                              static_cast<int>(WindowHeight));
        Model& model = binder.GetModel();
        if (model.textPx == textPx)
            return;
        model.textPx = textPx;
        binder.MarkDirty("text_px");
    }
}
