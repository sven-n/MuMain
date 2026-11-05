#pragma once

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
// The world is pinned: WorldTime per situation, the draw frame of Random::DrawRand, and one OBJECT for every drawing,
// set up by the game's ItemObjectAttribute.
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

    // "+7 at 1.25 s, doppelganger, fading", for messages and the baseline.
    [[nodiscard]] std::string Describe() const;
};

// The situations every item of a look is drawn in: a plain drawing, and from it one change at a time (the levels of
// the level glow, other clock times, a doppelganger, a monster's hands, fading, without the skin, excellent).
std::vector<Situation> Situations();

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

    // Opens the models of these items.
    void Open(std::span<const int> itemTypes);

private:
    struct State;
    State* m_state;
};

// The item types whose shipped model entry names this render style, in item type order.
std::vector<int> ItemsUsing(std::string_view renderStyle);

// "(0,13)".
std::string DescribeItem(int itemType);

// Draws the item as the game draws it, in the situation, and records the drawing.
Record RecordItem(int itemType, const Situation& situation);

// Draws the item with the render style of its name taken from the code: the data look of that name is left out of
// g_LookStore while it draws.
Record RecordItemWithCode(int itemType, const Situation& situation, std::string_view renderStyle);

// The first difference between a drawing with the old code and one with the data, in words ("draw 2 of 4, mesh 1:
// blendMeshAlpha 0.812 (code) vs 0.79 (data)"); empty when they are equal.
std::string FirstDifference(const Record& code, const Record& data);

// A digest of the mesh draws and of what the drawing left on the model, with the numbers rounded to 6 digits, for the
// baseline: GPU calls carry vertex hashes that differ between compilers, and the drawing of one mesh draw is the same
// code for every look.
std::string Digest(const Record& record);
} // namespace LookRecorder
