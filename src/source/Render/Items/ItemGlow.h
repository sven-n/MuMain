#pragma once

#include "Data/GameData/ItemData/ItemModelDefinition.h"

#include <array>

class BMD;
class OBJECT;

// How items glow: the values of item models come from the item model files
// (Data/Items/Models, "glow"), their colors from the glow color list
// (Data/Effects/GlowColors.json). Which glow an item gets for its level,
// excellent options or ancient set stays in the drawing code.
namespace Render::Items::Glow
{
using Color = std::array<float, 3>;
using Colors = Data::Items::ItemGlowColors;

// The glow of the item model, or the default glow for other models.
const Data::Items::ItemGlow& Get(int modelType);

// The glow colors of the item model, or of the item a model is drawn for
// (the inventory models of the Rage Fighter armor, the second models of the
// Rage Fighter gloves). Other models (monsters, ...) keep the colors the
// drawing code always had.
const Colors& GetColors(int modelType);

// The level the model glows like instead of the item level.
int GetLevel(int modelType, int level);

// Whether excellent items with this model glow.
bool HasExcellentGlow(int modelType);

// Draws the glow pass of items +7 and up.
void RenderGlow(BMD* b, OBJECT* o, int modelType, int renderType, float alpha, int texture);

// Draws the shine of items +11 and up, or the glow of ancient items.
void RenderShine(BMD* b, OBJECT* o, int modelType, int renderType, float alpha, int texture);

// Draws the excellent glow of the model.
void RenderExcellentGlow(BMD* b, OBJECT* o, int modelType, float alpha);
} // namespace Render::Items::Glow
