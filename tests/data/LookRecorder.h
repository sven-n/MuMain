#pragma once

#include "Data/GameData/LookData/LookDefinition.h"

#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// The draw recorder of looks in data (design L25 and L32 in docs/superpowers/specs/2026-11-01-looks-in-data-design.md):
// draws a real item of the test data with the game's own item drawing (RenderPartObject, which draws the render style
// and then the level glow), through a drawer that writes down each mesh draw and hands it on to the normal drawing,
// which draws into Render::Drawing::DrawCallLog in place of the renderer. A look moved from code into data is drawn
// with its old code and with the data in the same build, and both records must be equal: the mesh draws first, then
// the GPU calls.
//
// The world is pinned: WorldTime per situation, the key of Random::DrawRand, the model's scrolling mesh, and one
// OBJECT for every drawing, set up by the game's ItemObjectAttribute.
namespace LookRecorder
{
// How an item is drawn.
struct Situation
{
    int level = 0;
    // Milliseconds of WorldTime.
    double worldTime = 0.0;
    // RENDER_TEXTURE, or with RENDER_DOPPELGANGER or RENDER_EXTRA (in a monster's hands).
    int renderType = 0;
    float alpha = 1.f;
    bool hideSkin = false;
    bool excellent = false;
    // The Infinity Arrow buff on the drawn object.
    bool infinityArrow = false;
    // How far the object has turned (OBJECT::Angle[1]), in degrees.
    float turn = 0.f;
    // The light the drawing hands the item, a grey: the inventory's is white.
    float light = 1.f;
    // Drawn in the inventory (Render::Items::Looks::InventoryDrawing); hideSkin with it.
    bool inInventory = false;

    // "+7 at 1.25 s, doppelganger, fading", for messages and the baseline.
    [[nodiscard]] std::string Describe() const;
};

// The situations every item of a look is drawn in: a plain drawing, and from it one change at a time (the levels of
// the level glow, other clock times, a doppelganger, a monster's hands, fading, without the skin, excellent).
std::vector<Situation> Situations();

// The situations the items of a render style are drawn in: those of every look, and the ones the conditions of the look
// of that name in g_LookStore ask for (design L25): with the buff of a layer's condition, and the levels +1 to +6 for a
// condition on the level or the hidden mesh, which item effects set by level. A look with a layer only for the
// inventory or the ground is drawn as the game draws there: worn and on the ground in a grey light, and in the
// inventory, in white light, at +0, +1, +3 and +7.
std::vector<Situation> SituationsFor(std::string_view renderStyle,
                                     Data::Looks::LookKind kind = Data::Looks::LookKind::RenderStyle);

// One field of a mesh draw: its name and value as text.
using Field = std::pair<const char*, std::string>;

struct Record
{
    // One entry per mesh draw, its fields in a fixed order. The texture bit of a reflection pass is left out of the
    // flags: it means nothing there (BMD::DrawMesh reads it only for a pass without a reflection).
    std::vector<std::vector<Field>> meshDraws;
    // The calls that reached the renderer (DrawCallLog), with the state each was drawn with.
    std::vector<std::string> gpuCalls;
    // What the drawing left on the model: its light, scrolling mesh and lighting switch.
    std::vector<Field> modelAfter;
};

// The model slots, item models and glow colors of the test data for as long as it lives: the shipped item model
// database, with the models of the given items opened (with their textures, without a GPU).
class ShippedItems
{
public:
    ShippedItems();
    ~ShippedItems();
    ShippedItems(const ShippedItems&) = delete;
    ShippedItems& operator=(const ShippedItems&) = delete;

    // Opens the models of these items (and part models), then, as the game does after the item models, loads the item
    // effect textures and finds the textures the looks in data name; open them after the looks are in g_LookStore.
    void Open(std::span<const int> itemTypes);

private:
    struct State;
    State* m_state;
};

// What a look is drawn on: an item type, or a part model (design L28) as PartModelDrawn of its slot.
constexpr int PartModelDrawn(int modelSlot)
{
    return -1 - modelSlot;
}

// The item types whose shipped model entry names this render style (or item effect), in item type order, then the
// part models whose shipped entry names the render style (PartModelDrawn), in the order of PartModelSlots.
std::vector<int> ItemsUsing(std::string_view renderStyle,
                            Data::Looks::LookKind kind = Data::Looks::LookKind::RenderStyle);

// "(0,13)", or a part model's name in brackets: "(phoenixSoulStarWing)".
std::string DescribeItem(int itemType);

// Draws the item as the game draws it, in the situation, and records the drawing.
Record RecordItem(int itemType, const Situation& situation);

// Draws the item with the render style (or item effect) of its name taken from the code: the data look of that name
// and kind is left out of g_LookStore while it draws.
Record RecordItemWithCode(int itemType, const Situation& situation, std::string_view renderStyle,
                          Data::Looks::LookKind kind = Data::Looks::LookKind::RenderStyle);

// How far a number of a mesh draw may be from the code's when every GPU call is the same (design L34): a pulse the
// code wrote as (sin + 1.2) * 0.4 and the data draws as sin * 0.4 + 0.48 rounds in another order, a few float steps
// (units in the last place) away.
constexpr int AllowedFloatSteps = 8;

// The first difference between a drawing with the old code and one with the data, in words ("draw 2 of 4, mesh 1:
// blendMeshAlpha 0.812 (code) vs 0.79 (data)"); empty when they are equal: the same GPU calls, and the same mesh
// draws and model after the drawing, their numbers at most AllowedFloatSteps apart.
std::string FirstDifference(const Record& code, const Record& data);

// A digest of the mesh draws and of what the drawing left on the model, for the baseline: GPU calls carry vertex hashes
// that differ between compilers, and the drawing of one mesh draw is the same code for every look.
// "7 <hash> ... <hash>": the number of mesh draws, then a hash of the numbers in steps of 0.0001 on each of five grids
// whose edges are a fifth of a step apart. A number a float step of a sine away (macOS's sine, a fused multiply and
// add) crosses the edge of one grid at most, so a drawing keeps a hash unless five of its numbers each sit at another
// grid's edge.
std::string Digest(const Record& record);
// Whether a drawing's Digest is the baseline's: the same number of mesh draws, and one of the hashes the same.
bool SameDigest(const std::string& digest, const std::string& baseline);
} // namespace LookRecorder
