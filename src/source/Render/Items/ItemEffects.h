#pragma once

#include <string_view>

class BMD;
class OBJECT;

// What item models do besides being drawn: sprites, particles and lightning
// on their bones, a pulsing glow mesh, a mesh hidden by level, their own
// drawing instead of the plain one, the shine of some items below +3. Each
// effect is code with a name; the model entry of an item names its effect
// (Data/Items/Models, "effect"). Items with the same effect share it.
namespace Render::Items::Effects
{
// Whether there is an effect of this name. The model loader checks the names
// of the model entries with it.
bool Exists(std::string_view name);

enum class Result
{
    // The item has no effect that runs before the model is drawn.
    None,
    // The effect ran; the model is drawn as usual.
    Applied,
    // The effect drew the model itself; nothing more is drawn.
    Drawn,
};

// Runs the effect of the item before its model is drawn (the effects are
// taken from the item model database after each build of it). It may change
// the level the model glows like.
Result Apply(BMD* b, OBJECT* o, int modelType, float alpha, int& level);

// Draws the model below +3 for the effects that shine then, instead of the
// plain drawing. False for the others.
bool RenderBelowPlus3(BMD* b, OBJECT* o, int modelType, float alpha, int renderType, float* light);
} // namespace Render::Items::Effects
