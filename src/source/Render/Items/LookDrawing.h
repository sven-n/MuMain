#pragma once

#include "Data/GameData/LookData/LookDefinition.h"
#include "Render/Models/DrawLook.h"

class BMD;
class OBJECT;

// Draws the looks of items that are data (Data/Effects/RenderStyles.json): each layer becomes the model draws the
// looks in code make, so a look drawn from data and its old code draw the same.
namespace Render::Items::Looks
{
// Draws a render style's layers in their order, then leaves its item values in `look` for what is drawn after it
// (the level glow). `alpha` and `renderType` are the drawing's own.
//
// A layer of every mesh draws with RenderBody (the texture scripts, leaving out the item's hidden mesh); a layer of
// every mesh but one draws with RenderBody leaving that one out; a layer of listed meshes draws each with RenderMesh.
// A layer's light is put on the model and stays for the layers after it; its scrolling mesh is put on the model while
// it draws, and the model keeps the one it had unless the item values set one.
void DrawRenderStyle(const Data::Looks::LookDefinition& style, BMD* b, const OBJECT* o, DrawLook& look, float alpha,
                     int renderType);

// Finds the texture each layer names, loading the ones no code loaded; done after the item textures are loaded. A layer
// whose texture is missing is skipped, and that is logged as an error.
void PrepareTextures();
// The same for one look when the looks change (an edit, an undo): the textures not found yet, also those missing
// before, as their file may have come since. A missing texture is logged once.
void PrepareTextures(const Data::Looks::LookDefinition& style);
// The same for a look the Style Editor shows before it is applied, when it changes; a missing texture is not logged
// (the look's problems name it).
void PrepareDraftTextures(const Data::Looks::LookDefinition& style);
} // namespace Render::Items::Looks
