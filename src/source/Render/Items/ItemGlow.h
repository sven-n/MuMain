#pragma once

#include "Data/GameData/ItemData/ItemModelDefinition.h"

class BMD;
class OBJECT;

// How items glow: the values of item models come from the item model files
// (Data/Items/Models, "glow"). Which glow an item gets for its level,
// excellent options or ancient set stays in the drawing code.
namespace Render::Items::Glow
{
// The glow of the item model, or the default glow for other models.
const Data::Items::ItemGlow& Get(int modelType);

// The glow of the item model, or of the item a model is drawn for (the
// inventory models of the Rage Fighter armor, the second models of the Rage
// Fighter gloves); the default glow for other models.
const Data::Items::ItemGlow& GetOfDrawnItem(int modelType);

// The level the model glows like instead of the item level.
int GetLevel(int modelType, int level);

// Whether excellent items with this model glow.
bool HasExcellentGlow(int modelType);

// Draws the model with a glow pass on the given meshes.
void RenderMeshes(BMD* b, OBJECT* o, const Data::Items::ItemGlowMeshes& meshes, int renderType, float alpha,
                  int texture);

// Draws the excellent glow of the model.
void RenderExcellentGlow(BMD* b, OBJECT* o, int modelType, float alpha);
} // namespace Render::Items::Glow
