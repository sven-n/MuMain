#pragma once

#include <string>
#include <vector>

namespace Data::Items
{
// A model that several items use (Data/Items/Models/SharedModels.json): one
// .bmd file, opened once. The model entries of these items name it with
// "model" instead of having a file of their own; their inventory and ground
// display, glow, render style and item effect stay their own.
struct SharedItemModel
{
    // A name of letters and digits: "skillParchment".
    std::string name;
    // Relative to the client folder, with '/': "Data/Item/rollofpaper.bmd".
    std::string file;
    // Folders below Data/ with the textures of the model.
    std::vector<std::string> textureFolders;
    // Indexes of the meshes that are drawn without blending.
    std::vector<int> noneBlendMeshes;

    bool operator==(const SharedItemModel&) const = default;
};
} // namespace Data::Items
