#pragma once

#include <string_view>

class BMD;
class OBJECT;

// The looks of item models that are more than a plain textured model: chrome
// layers, glowing meshes, animated textures, ... Each look is code, a render
// style with a name; the model entry of an item names its style
// (Data/Items/Models, "renderStyle"). Items with the same look share a style.
namespace Render::Items::Styles
{
// Whether there is a render style of this name.
bool Exists(std::string_view name);

// Takes the render styles of the item models from the item model database.
// Called when the models are opened; names that are not styles are drawn
// plainly (the model loader reports them).
void Assign();

// Draws the model with the render style of its item. False when the item has
// no style, or its style is not for this drawing (e.g. a doppelganger); the
// drawing code then draws the model itself.
bool Render(BMD* b, OBJECT* o, int modelType, float alpha, int renderType);

// Draws the glow pass of items +7 and up for the few render styles that glow
// differently from their "glow" values. False for the others.
bool RenderGlow(BMD* b, OBJECT* o, int modelType, int renderType, float alpha, int texture);
} // namespace Render::Items::Styles
