#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace Rml
{
class DataModelConstructor;
}

// A native item grid (CInventoryCtrl) as its document draws it: one entry per cell, in row order.
// The theme's item_grid.rcss styles the classes; the items themselves are 3D, drawn into the
// window's render target over the cells.
namespace UI::Items
{
struct ItemGridCell
{
    // Under an item: "tint-normal", "tint-durability-50", "-70", "-80", "-100", "tint-untradeable".
    Rml::String tint;
    // Under the item on the cursor: "drop-free" (an empty cell), "drop-blocked" (empty, but the
    // window refuses it), "drop-valid" (an item it acts on: a jewel, a stack), "drop-invalid".
    Rml::String drop;
    // An item's stack count, on its top-right cell.
    Rml::String count;

    bool operator==(const ItemGridCell&) const = default;
};
using ItemGridCells = std::vector<ItemGridCell>;

// Registers ItemGridCell and its array on a window's data model, once per constructor.
void RegisterItemGridCells(Rml::DataModelConstructor& c);
} // namespace UI::Items
