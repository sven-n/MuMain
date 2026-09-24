#pragma once

#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/Scaling/UITransform.h"

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
    template <typename Model>
    void SyncRootTransform(RmlModelBinder<Model>& binder, const POINT& pos)
    {
        const auto transform = UI::Scaling::GetActiveTransform();
        Model& model = binder.GetModel();
        model.rootX = static_cast<float>(pos.x) * transform.scaleX + transform.offsetX;
        model.rootY = static_cast<float>(pos.y) * transform.scaleY + transform.offsetY;
        model.rootScale = transform.scaleX;
        binder.MarkDirty("root_x");
        binder.MarkDirty("root_y");
        binder.MarkDirty("root_scale");
    }

    // Legacy-theme text that must match the native text renderer's size (and be rasterised at it)
    // takes this physical size and counter-scales itself out of the root transform -- see
    // character_info.rml's header comment. `Model` must expose `float textPx` bound to "text_px".
    template <typename Model> void SyncNativeTextSize(RmlModelBinder<Model>& binder)
    {
        const float textPx =
            UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, UI::Scaling::GetActiveTransform());
        Model& model = binder.GetModel();
        if (model.textPx == textPx)
            return;
        model.textPx = textPx;
        binder.MarkDirty("text_px");
    }
}
