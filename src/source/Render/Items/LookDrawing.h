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
// (the level glow). `alpha` and `renderType` are the drawing's own. False, drawing nothing, for a drawing the look is
// not for (its Not for); the game then draws the item plainly. A layer whose Only when doesn't hold is skipped.
//
// A layer of every mesh draws with RenderBody (the texture scripts, leaving out the item's hidden mesh); a layer of
// every mesh but one draws with RenderBody leaving that one out; a layer of listed meshes draws each with RenderMesh.
// A layer's light is put on the model and stays for the layers after it; its scrolling mesh is put on the model while
// it draws, and the model keeps the one it had unless the item values set one.
bool DrawRenderStyle(const Data::Looks::LookDefinition& style, BMD* b, const OBJECT* o, DrawLook& look, float alpha,
                     int renderType);

// The value of a shape that follows the clock (wave, bounce, scroll, ramp, zigzag, frames) at the clock `time`, in
// milliseconds like WorldTime; `from` for a shape that doesn't follow it (random, turn). The Style Editor's curves draw
// with it, so they show what the game draws.
float ShapeAtClock(const Data::Looks::Shape& shape, double time);

// The model drawing's flag of a reflection's motion (RENDER_CHROME, RENDER_CHROME2, ...).
int RenderFlagOf(Data::Looks::Motion motion);

// While one lives, items are drawn in the inventory (RenderObjectScreen): a layer's place "inventory" (design L16).
class InventoryDrawing
{
public:
    InventoryDrawing();
    ~InventoryDrawing();
    InventoryDrawing(const InventoryDrawing&) = delete;
    InventoryDrawing& operator=(const InventoryDrawing&) = delete;

    [[nodiscard]] static bool Active();
};

// Sets the item values an item effect that is data sets before the drawing (design L13): each entry whose Only when
// holds, in their order; an entry can also set the opacity the drawing hands the look (`alpha`) and the level the
// item glows like (`level`, which the later entries' conditions read).
void ApplyItemEffect(const Data::Looks::LookDefinition& effect, BMD* b, const OBJECT* o, DrawLook& look, float& alpha,
                     int& level);
// Puts the light of an item effect that is data on the item's object, in the update (design L45).
void SpawnItemEffect(const Data::Looks::LookDefinition& effect, OBJECT* o);

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
