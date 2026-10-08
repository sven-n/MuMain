#pragma once

namespace Rml
{
class Element;
class ElementDocument;
}

namespace UI::RmlBridge
{
// True while RmlUi hovers an element of `document` other than its body: the pointer is over what
// the document draws and takes pointer events on. A window drawn by a document asks this instead
// of testing MouseX/MouseY against a box in its own units. The theme decides what takes pointer
// events (a panel blocks the world with `pointer-events: auto`).
bool IsPointerOver(Rml::ElementDocument* document);
// True while RmlUi hovers `element` or one of its descendants, in a visible document.
bool IsPointerOver(Rml::Element* element);
} // namespace UI::RmlBridge
