#pragma once

#include <RmlUi/Core/Types.h>

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
// True while `point` (window pixels) lies inside `element`'s drawn border box, its transforms
// included, in a visible document -- also where the element takes no pointer events (a HUD part the
// world stays clickable through, whose own rectangle still holds the pointer).
bool IsPointWithin(Rml::Element* element, Rml::Vector2f point);
// IsPointWithin() at the game's pointer.
bool IsPointerWithin(Rml::Element* element);
// The game's pointer in `element`'s own layout units, from its border box's top-left, through every
// transform on it and its ancestors -- where on a map or a board the pointer is. False while the
// element is not drawn.
bool PointerIn(Rml::Element* element, Rml::Vector2f& local);
} // namespace UI::RmlBridge
