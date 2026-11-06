#pragma once

#ifdef _EDITOR

#include "Data/GameData/LookData/LookDefinition.h"
#include "Data/GameData/LookData/LookEffects.h"

#include <string>

// The fields of a look in the Style Editor (design L19), in designer terms (L8, L9, L10): a layer's kind, motion,
// blend, meshes, texture, light and values, and the item values a look leaves after drawing. A value that changes over
// time names a shape and draws its curve below its fields. Each returns true in the frame the user changed something.
namespace MuEditor::Looks::Fields
{
bool Layer(Data::Looks::Layer& layer);
bool ItemValues(Data::Looks::ItemValues& values);
// A layer as its effect (design L35, L40): its meshes, then only the fields of its main effect and of what else it
// does, as `effect` describes it (a pulse's strength with its curve, a shine's motion, an overlay's texture).
bool Effect(Data::Looks::Layer& layer, const Data::Looks::LayerEffect& effect);

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
} // namespace MuEditor::Looks::Fields

#endif // _EDITOR
