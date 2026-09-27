#pragma once

namespace Rml
{
class Context;
} // namespace Rml

// The original client lays its dialogs and NPC menus out on a 640x480 canvas
// (UI::Scaling::PanelTransform): one uniform scale, capped, and the canvas centred in the window.
// A dialog that a theme places from the canvas top (legacy: `top: 100dp`) therefore sits lower
// than the same `dp` offset from the window top whenever the canvas does not fill the window
// height (the scale hit its cap, or the window is taller than 4:3).
//
// Dialogs bind this value as `canvas_top`; a theme that places them like the original adds it to
// the panel's margin-top (`data-style-margin-top="canvas_top + 'dp'"`), others ignore it.
namespace UI::RmlBridge
{
// The dialog canvas's top edge below the window top, in `dp` of the context (its window size and
// dp ratio). 0 while the canvas fills the window height.
float DialogCanvasTop(const Rml::Context* context);
} // namespace UI::RmlBridge
