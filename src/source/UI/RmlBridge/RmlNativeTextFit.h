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
// An element opts in with `class="text-fit"` (any theme) or `class="native-fit"` (only themes
// declaring NativeTextSize). Its content box is the text box, its text is one line, and its
// font-size is inherited from its parent; this pass owns its own font-size and sets it in `em`.
// Both opt-ins fit by font size, so glyphs are never geometrically stretched. `text-fit` can shrink
// to 1px; `native-fit` keeps the original minimum font size.
namespace UI::RmlBridge
{
// Fits every opted-in element of the context's visible documents to its box. Call after the
// context's Update() (layout done); an element is measured again only when its text, box width,
// inherited font size changes (also typography for text-fit). A changed document is laid out again before drawing.
// text-fit elements are followed from their creation, so the pass does not walk every document: the
// class must be set in markup or before the element's first frame. native-fit walks the documents
// of themes declaring NativeTextSize.
void FitNativeTextToBoxes(Rml::Context* context);
} // namespace UI::RmlBridge
