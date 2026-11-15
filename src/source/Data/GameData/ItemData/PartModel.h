#pragma once

#include "Data/GameData/ItemData/SharedItemModel.h"

#include <string>

namespace Data::Items
{
// A model that is no item but is drawn with items (Data/Items/Models/PartModels.json, design L28): the decorations of
// the +15 armor, the wing of the held Phoenix Soul Star, the inventory model of a Rage Fighter armor. The entry holds
// its file, texture folders and render style; when and where it is drawn, and in which model slot, stays in code
// (PartModelSlots.h).
struct PartModel
{
    // The name, file, texture folders, none-blend meshes and mesh names, as a shared model has them.
    SharedItemModel model;
    // A look of Data/Effects/RenderStyles.json; empty: the model is drawn plainly.
    std::string renderStyle;

    bool operator==(const PartModel&) const = default;
};
} // namespace Data::Items
