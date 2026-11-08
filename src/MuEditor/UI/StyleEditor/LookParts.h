#pragma once

#ifdef _EDITOR

#include "Data/GameData/LookData/LookDefinition.h"

#include <cstddef>
#include <string>
#include <vector>

// The parts list of the Style Editor (design L35, L38): the meshes of the item the preview draws, each with its name
// and texture, and the layers of the look on each.
namespace MuEditor::Looks
{
// The row of every part: the layers of every mesh, or of all but some.
inline constexpr int EveryPart = -1;
// The base row: the layers of the item's own texture, on whatever meshes (design L40).
inline constexpr int BasePart = -2;

struct LookPart
{
    int mesh = EveryPart;
    // The name the model gives the mesh (meshNames); empty for none.
    std::string name;
    std::string texture;
};

// The meshes of the item's loaded model with their names; none while it is not loaded.
std::vector<LookPart> PartsOf(int itemType);

// The layers of the look on the part, in drawing order: those that list the mesh, or for EveryPart those of every mesh
// and of every mesh but some.
std::vector<std::size_t> LayersOn(const Data::Looks::LookDefinition& look, int mesh);

// "Crystal", or "1 · wand44_R.jpg" for a part without a name.
std::string PartLabel(const LookPart& part);
} // namespace MuEditor::Looks

#endif // _EDITOR
