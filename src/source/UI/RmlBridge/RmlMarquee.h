#pragma once

namespace Rml
{
class Context;
} // namespace Rml

// Marquee labels: text that keeps one line in a constricted box and shows the rest on hover.
//
// An element opts in with `class="marquee"` (base.rcss: one line, clipped to its box); its text is
// its first child. While that text is wider than the box, it shows as much as fits followed by
// "..". While the pointer is within the box, the full text shows instead, carrying
// `marquee-overflow`, and scrolls to its end and back; the pointer leaving puts the shortened text
// back. The pass tests the box itself, so a label that lets the mouse through to the control under
// it still marquees. Text set by the document or its model is taken as the new full text.
namespace UI::RmlBridge
{
// Updates every marquee of the context's visible documents. Call after the context's Update().
void UpdateMarquees(Rml::Context* context);
} // namespace UI::RmlBridge
