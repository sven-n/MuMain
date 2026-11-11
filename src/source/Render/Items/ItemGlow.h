#pragma once

#include "Data/GameData/EffectData/Shines.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"

#include <array>

#include "Render/Models/DrawLook.h"

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

// The glow of the item model (the second models of the Rage Fighter gloves
// take the glow of their glove), or the default glow for other models.
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

// The shine below +3 the glow of the model names (design L30), or nullptr for the models drawn plainly below +3.
const Data::Effects::Shine* FindShineBelowPlus3(int modelType);

// The model drawing's flags of a pass of a shine below +3, without the drawing's own.
int RenderTypeOf(const Data::Effects::ShinePass& pass);

// Draws the glow pass of items +7 and up; a model whose glow draws a mesh again in white then keeps that mesh as its
// glowing mesh (`look`).
void RenderGlow(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType, int texture);

// Draws the shine of items +11 and up, or the glow of ancient items.
void RenderShine(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType, int texture);

// Draws the excellent glow of the model.
void RenderExcellentGlow(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha);
} // namespace Render::Items::Glow
