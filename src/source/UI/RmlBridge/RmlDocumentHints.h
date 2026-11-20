#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace Rml
{
class Context;
}

// A hint written in a document: `data-hint="text"` on an element (bound text:
// data-attr-data-hint="x"), shown in the shared tooltip (RmlTooltip.h) as CNewUIButton drew a
// button's hint -- above the element, below it when there is no room above -- while the pointer
// is over the element or any of its children. A line break in the text starts a new line, so a
// long hint stays a readable width. An empty attribute shows nothing. For a hint whose
// lines or colours come from game state, use ElementTooltip (RmlElementTooltip.h).
namespace UI::RmlBridge::DocumentHints
{
// Once a frame, before the context updates: shows the hovered element's hint, or hides it.
void Update(Rml::Context& context);

// The lines a hint shows: its text split at each line break.
std::vector<Rml::String> HintLines(const Rml::String& hint);
} // namespace UI::RmlBridge::DocumentHints
