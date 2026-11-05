#pragma once

#ifdef _EDITOR

#include "Data/GameData/LookData/LookDefinition.h"

#include <string>

// The fields of a look in the Style Editor (design L19), in designer terms (L8, L9, L10): a layer's kind, motion,
// blend, meshes, texture, light and values, and the item values a look leaves after drawing. A value that changes over
// time names a shape and draws its curve below its fields. Each returns true in the frame the user changed something.
namespace MuEditor::Looks::Fields
{
bool Layer(Data::Looks::Layer& layer);
bool ItemValues(Data::Looks::ItemValues& values);

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
