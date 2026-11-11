#pragma once

#ifdef _EDITOR

#include "Data/GameData/LookData/LookDefinition.h"
#include "Data/GameData/LookData/LookEffects.h"
#include "LookParts.h"

#include <string>
#include <vector>

// The fields of a look in the Style Editor (design L19), in designer terms (L8, L9, L10): a layer's kind, motion,
// blend, meshes, texture, light and values, and the item values a look leaves after drawing. A value that changes over
// time names a shape and draws its curve below its fields. Each returns true in the frame the user changed something.
// The numbers are drag fields, every field says in a tooltip what it does, the meshes are the picked item's `parts` by
// name (checkboxes, a dropdown) and a light's color has a picker (design L42); without parts a mesh is a number.
namespace MuEditor::Looks::Fields
{
bool Layer(Data::Looks::Layer& layer, const std::vector<LookPart>& parts = {});
// The look's own options: Not for (doppelgangers, outside a monster's hands) and Draws alone (design L47).
bool LookOptions(Data::Looks::LookDefinition& look);
// `light`: whether the values can set the light; the values an item effect sets before the drawing can't.
bool ItemValues(Data::Looks::ItemValues& values, const std::vector<LookPart>& parts = {}, bool light = true);
// An item effect's entry before the drawing (design L13): when it holds, the item values it sets, the drawing's
// opacity and the level the item glows like from then on.
bool ItemValuesBeforeEntry(Data::Looks::ItemValuesBefore& entry, const std::vector<LookPart>& parts = {});
// An item effect's light of the object (design L45): none, or red, green and blue, each a number or a shape.
bool ObjectLight(std::optional<std::array<Data::Looks::Number, 3>>& light);
// One line for an entry of the item effect's list: "Only when: glow mesh, glows like level +7".
std::string DescribeItemValuesBefore(const Data::Looks::ItemValuesBefore& entry);
// A layer as its effect (design L35, L40): its meshes, then only the fields of its main effect and of what else it
// does, as `effect` describes it (a pulse's strength with its curve, a shine's motion, an overlay's texture).
bool Effect(Data::Looks::Layer& layer, const Data::Looks::LayerEffect& effect, const std::vector<LookPart>& parts = {});

// A mesh the mesh fields offer: its number and its label.
struct MeshOption
{
    int mesh = 0;
    std::string label;

    bool operator==(const MeshOption&) const = default;
};
// The parts in their order ("0 · Crystal", "1 · wand44_R.jpg" without a name), then the listed meshes the parts
// don't have, sorted ("5 (not in the model)").
std::vector<MeshOption> MeshOptions(const std::vector<LookPart>& parts, const std::vector<int>& listed);
// `meshes` with `mesh` added at its end or taken out; the others keep their order.
std::vector<int> WithMesh(std::vector<int> meshes, int mesh, bool in);

// "Pulse", and the gallery's line about it ("" for the base).
const char* EffectName(Data::Looks::EffectKind kind);
const char* EffectNote(Data::Looks::EffectKind kind);
// "Shine (glows, flows)": the main effect and what else the layer does.
std::string EffectLabel(const Data::Looks::LayerEffect& effect);
// "every mesh but 1", "meshes 0, 2".
std::string DescribeMeshes(const Data::Looks::MeshSet& meshes);

// One line written from a layer's fields, for the layer list: "Reflection: slow drift, glow, every mesh but 2; texture,
// glow mesh".
std::string DescribeLayer(const Data::Looks::Layer& layer);
// The item values a look leaves, for the last line of the layer list; empty when it leaves none.
std::string DescribeItemValues(const Data::Looks::ItemValues& values);

// A shape's value at `seconds` (the clock in the editor's terms), as the curve draws it; a random shape gives its
// middle.
double ShapeValueAt(const Data::Looks::Shape& shape, double seconds);

// The curve under a shape's fields (design L43) shows this many seconds of the preview's clock.
inline constexpr double CurveSpan = 10.0;
// The preview's clock in seconds, which the curves mark; the Style Editor sets it each frame before the fields.
void SetCurveTime(double seconds);
// What a curve shows at the clock `now`: the ten seconds from the last whole ten before it, its values from `low`
// (at most 0) to `high` (at least 1), and where `now` is in it.
struct CurveWindow
{
    double start = 0.0;
    double low = 0.0;
    double high = 1.0;
    double now = 0.0;
};
CurveWindow CurveWindowOf(const Data::Looks::Shape& shape, double now);
// The shape's value `seconds` into the window; a scroll from where the window starts, as a texture looks the same
// whatever whole steps it moved before.
double CurveValueAt(const Data::Looks::Shape& shape, const CurveWindow& window, double seconds);
} // namespace MuEditor::Looks::Fields

#endif // _EDITOR
