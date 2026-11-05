#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemModelDefinition.h"

#include <cstdint>
#include <string>
#include <vector>

class BMD;

namespace MuEditor::Items
{
// One mesh of an item's loaded model, as the mesh list of the Item Model window shows it.
struct ItemMeshRow
{
    int mesh = 0;
    std::string texture;
    // Never drawn: its texture name starts with "hid", or the _H script.
    bool hidden = false;
    // The _R script: drawn glowing.
    bool glows = false;
    // The _S script: its texture scrolls.
    bool scrolls = false;
    // The _N script or a none-blend mesh: the chrome-type passes and the level glow skip it.
    bool noChrome = false;
    // Its texture has pixels the alpha test drops.
    bool holes = false;
    int vertices = 0;
    int triangles = 0;
    // The texture as it was loaded: its file and the size the game works with; empty and 0 when it is not loaded.
    std::string textureFile;
    int textureWidth = 0;
    int textureHeight = 0;
};

// The fields of a model entry that name a mesh, as bits.
namespace MeshUse
{
enum : std::uint8_t
{
    NoneBlend = 1 << 0,
    Glow = 1 << 1,
    GlowAllBut = 1 << 2,
    Shine = 1 << 3,
    ShineAllBut = 1 << 4,
    Excellent = 1 << 5,
    ExcellentWithoutSkin = 1 << 6,
};
} // namespace MeshUse
using MeshUses = std::uint8_t;

// The rows of the model's meshes, in mesh order.
[[nodiscard]] std::vector<ItemMeshRow> DescribeMeshes(const BMD& model);
// The fields of `entry` that name `mesh`.
[[nodiscard]] MeshUses FindMeshUses(const Data::Items::ItemModelDefinition& entry, int mesh);
} // namespace MuEditor::Items

#endif // _EDITOR
