#pragma once

#include "UI/RmlBridge/RmlModelBinder.h"

#include "UI/Scaling/UITransform.h"

extern unsigned int WindowWidth;
extern unsigned int WindowHeight;

// The native text renderer's sizes in screen pixels, for RmlUi text that must match it.
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

// The same for a box with a height too (UI::Scaling::NativeTextPixelSizeInBounds()).
inline float NativeTextPxInBounds(UI::Scaling::FontRole role, float measuredWidth, float measuredHeight, float boxWidth,
                                  float boxHeight)
{
    return UI::Scaling::NativeTextPixelSizeInBounds(role, static_cast<int>(WindowWidth), static_cast<int>(WindowHeight),
                                                    measuredWidth, measuredHeight, boxWidth, boxHeight);
}

    // Legacy-theme text that must match the native text renderer's size (and be rasterised at it)
    // takes this physical size and counter-scales itself out of its panel's scale -- see
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
