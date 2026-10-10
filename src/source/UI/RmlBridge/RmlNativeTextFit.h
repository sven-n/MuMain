#pragma once

namespace Rml
{
class Context;
} // namespace Rml

// Shrink-to-fit text for the legacy theme. The original client's RenderText() with a box width
// (a CNewUIButton's label, a menu box's centred line, the NPC shop's caption) draws a text wider
// than its box smaller, until the smallest font size (UI::Scaling::FitTextPixelSizeToWidth()'s
// rule, FontScaleForBounds() in the native renderer); RmlUi has no such property.
//
// An element opts in with `class="native-fit"`: its content box is the original's text box, its
// text is one line (the theme sets `white-space: nowrap`), and its font-size is inherited from its
// parent -- the pass owns the element's own font-size and sets it in `em` when the text is too
// wide. Its px must be window px (a native-text document, or a label counter-scaled out of its
// panel), as the minimum size is. Inert unless the theme sizes text like the original
// (theme.ini [Capabilities] NativeTextSize=1, RmlNativeText.h).
namespace UI::RmlBridge
{
// Fits every opted-in element of the context's visible documents to its box. Call after the
// context's Update() (layout done); an element is measured again only when its text, its box
// width or its inherited font size changed, and a document whose sizes changed is laid out again
// at once, so the frame draws the fitted size.
void FitNativeTextToBoxes(Rml::Context* context);
} // namespace UI::RmlBridge
